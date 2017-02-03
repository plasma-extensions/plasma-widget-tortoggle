/*
 *   Copyright (C) 2017 by Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "torcontrol.h"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>

#include <klocalizedstring.h>

#include <PackageKit/Daemon>

#include <sys/stat.h>
#include <signal.h>

class torcontrol::Private {
public:
    Private(torcontrol* qq)
        : q(qq)
        , systemTor(true)
        , whatInit(UnknownInit)
        , torPid(-1)
        , status(Unknown)
    {
        findTorPid();
        setWhatSuProgram();
        updateStatus();
        setWhatInit();
    }
    torcontrol* q;
    bool systemTor;
    QString workingOn;

    enum InitSystem {
        UnknownInit = 0,
        SysVInit = 1,
        UpstartInit = 2,
        SystemDInit = 3
    };
    InitSystem whatInit;
    // Find out what init system is being used, so we can use the correct
    // invokation method for the system calls
    void setWhatInit() {
        whatInit = UnknownInit;
        QProcess test;
        test.start("/sbin/init --version");
        if(test.waitForStarted() && test.waitForFinished()) {
            if(test.readAll().contains("upstart")) {
                whatInit = UpstartInit;
            }
        }
        if(whatInit == UnknownInit) {
            test.start("systemctl");
            if(test.waitForStarted() && test.waitForFinished()) {
                if(test.readAll().contains("-.mount")) {
                    whatInit = SystemDInit;
                }
            }
        }
        if(whatInit == UnknownInit) {
            QFileInfo cron("/etc/init.d/cron");
            if(cron.exists() && !cron.isSymLink()) {
                whatInit = SysVInit;
            }
        }
        qDebug() << "Init system is:" << whatInit;
    }

    void setWhatSuProgram() {
        whatSuProgram = "kdesu";
        if(QProcess::execute(whatSuProgram, QStringList() << "--version") < 0) {
            whatSuProgram = "kdesudo";
            if(QProcess::execute(whatSuProgram, QStringList() << "--version") < 0) {
                // this is a problem, what do?
                qDebug() << "No functioning kdesu or kdesudo was found. Please remidy this situation by installing one or the other";
            }
        }
    }
    QString whatSuProgram;
    QString runPrivilegedCommand(QStringList args)
    {
        QString output;
        QProcess cmd;
        cmd.start(whatSuProgram, args);
        if(cmd.waitForStarted()) {
            if(cmd.waitForFinished()) {
                output = cmd.readAll();
            }
        }
        return output;
    }

    Q_PID torPid;
    void findTorPid() {
        // go through proc and see if anything there's tor
        QProcess findprocid;
        findprocid.start("pidof", QStringList() << "tor");
        if(findprocid.waitForStarted()) {
            if(findprocid.waitForFinished()) {
                QStringList data = QString(findprocid.readAll()).split('\n');
                if(data.first().length() > 0) {
                    torPid = data.first().toInt();
                }
                else {
                    torPid = -2;
                }
            }
        }
    }

    QTimer statusCheck;
    RunningStatus status;
    bool systemSanityCheck() {
        // check tor install
        QProcess torTest;
        torTest.start("tor", QStringList() << "--version");
        if(!(torTest.waitForStarted() && torTest.waitForFinished(1000))) {
//         if(true) {
            status = NoTor;
            // tor is not installed...
            return false;
        }
        // If we get to here, then the system is sane
        return true;
    }
    void updateStatus() {
        if(systemTor && whatInit == SystemDInit) {
            if(systemSanityCheck()) {
                QProcess statusCheck;
                statusCheck.start("/usr/sbin/service", QStringList() << "tor" << "status");
                if(statusCheck.waitForStarted() && statusCheck.waitForFinished()) {
                    QString data(statusCheck.readAll());
                    if(data.contains("Active: active")) {
                        status = Running;
                    }
                    else if(data.contains("Active: inactive")) {
                        status = NotRunning;
                    }
                    else {
                        status = Unknown;
                    }
                }
            }
        }
        else {
            if(torPid > 0) {
                status = Running;
                struct stat sts;
                QString procentry = QString("/proc/%1").arg(QString::number(torPid));
                if (stat(procentry.toLatin1(), &sts) == -1 && errno == ENOENT) {
                    // process doesn't exist
                    status = NotRunning;
                    torPid = -2;
                }
            }
            else if(torPid == -1) {
                if(systemSanityCheck()) {
                    // We've just started up, find out whether we've actually got a running tor instance or not...
                    findTorPid();
                }
            }
            else if(torPid == -2) {
                if(systemSanityCheck()) {
                    // We've already been stopped, or we checked, so we know our status explicitly
                    status = NotRunning;
                }
            }
        }
        emit q->statusChanged();
    }

    void endWorkingOn(QString message, bool isError = false) {
        if(isError) {
            qWarning() << "Work completed with an error!" << message;
        }
        else {
            qDebug() << "Work completed successfully:" << message;
        }
        workingOn = message;
        emit q->workingOnChanged();
        QTimer::singleShot(3000, q, [=](){ workingOn = ""; emit q->workingOnChanged(); });
    }
};

torcontrol::torcontrol(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , d(new Private(this))
{
    connect(&d->statusCheck, &QTimer::timeout, [=](){ d->updateStatus(); });
    d->statusCheck.setInterval(5000);
    d->statusCheck.start();
}

torcontrol::~torcontrol()
{
    delete d;
}

torcontrol::RunningStatus torcontrol::status() const
{
    return d->status;
}

void torcontrol::setStatus(torcontrol::RunningStatus newStatus)
{
    d->updateStatus(); // Firstly, let's make sure out information is actually up to date
    if(d->systemTor) {
        switch(newStatus) {
            case Running:
                // Start tor
                if(status() == NotRunning || status() == Unknown) {
                    if(d->whatInit == Private::SystemDInit) {
                        QProcess::execute("/usr/sbin/service", QStringList() << "tor" << "start");
                    }
                    else {
                        d->runPrivilegedCommand(QStringList() << "torctl" << "start");
                        d->findTorPid();
                    }
                }
                break;
            case NotRunning:
                // Stop tor
                if(status() == Running) {
                    if(d->whatInit == Private::SystemDInit) {
                        QProcess::execute("/usr/sbin/service", QStringList() << "tor" << "stop");
                    }
                    else {
                        d->runPrivilegedCommand(QStringList() << "torctl" << "stop");
                    }
                }
                break;
            case Unknown:
            default:
                // Do nothing, this doesn't make sense...
                qDebug() << "Request seting the status to Unknown? Not sure what that would mean at all...";
                break;
        }
    }
    else {
        // a user-local tor instance, for non-admins
        // if running, send sigterm, otherwise start process
        if(status() == Running && newStatus == NotRunning) {
            kill(d->torPid, SIGTERM);
            d->torPid = -2;
        }
        else if((status() == NotRunning || status() == Unknown) && newStatus == Running) {
            QProcess::startDetached("tor", QStringList(), QString(), &d->torPid);
            qDebug() << "started tor with pid" << d->torPid;
        }
    }
    d->updateStatus();
}

QString torcontrol::iconName() const
{
    QString icon;
    switch(status()) {
        case Running:
            icon = "process-stop";
            break;
        case NotRunning:
            icon = "system-run";
            break;
        case Unknown:
        default:
            icon = "unknown";
            break;
    }
    return icon;
}

QString torcontrol::buttonLabel() const
{
    QString text;
    switch(status())
    {
        case Running:
            text = i18n("Stop TOR");
            break;
        case NotRunning:
            text = i18n("Start TOR");
            break;
        case Unknown:
        default:
            text = i18n("Please wait...");
            break;
    }
    return text;
}

bool torcontrol::systemTor() const
{
    return d->systemTor;
}

void torcontrol::setSystemTor(bool newValue)
{
    d->systemTor = newValue;
    d->findTorPid();
    d->updateStatus();
    emit systemTorChanged();
}

QString torcontrol::workingOn() const
{
    return d->workingOn;
}

void torcontrol::installTOR()
{
    qDebug() << "Attempting to install TOR";
    d->workingOn = i18n("Installing TOR");
    emit workingOnChanged();

    QStringList packages = QString("tor").split(" ");
    auto resolveTransaction = PackageKit::Daemon::global()->resolve(packages, PackageKit::Transaction::FilterArch);
    Q_ASSERT(resolveTransaction);

    connect(resolveTransaction, &PackageKit::Transaction::percentageChanged, resolveTransaction, [this, resolveTransaction](){
        if(resolveTransaction->percentage() < 101) {
            d->workingOn = i18n("Installing TOR (%1%)").arg(QString::number(resolveTransaction->percentage()));
        }
        else {
            d->workingOn = i18n("Installing TOR...");
        }
        emit workingOnChanged();
    });
    QHash<QString, QString>* pkgs = new QHash<QString, QString>();
    QObject::connect(resolveTransaction, &PackageKit::Transaction::package, resolveTransaction, [pkgs](PackageKit::Transaction::Info info, const QString &packageID, const QString &/*summary*/) {
        if (info == PackageKit::Transaction::InfoAvailable) {
            pkgs->insert(PackageKit::Daemon::packageName(packageID), packageID);
        }
        qDebug() << "resolved package"  << info << packageID;
    });

    QObject::connect(resolveTransaction, &PackageKit::Transaction::finished, resolveTransaction, [this, pkgs, resolveTransaction](PackageKit::Transaction::Exit status) {
        if (status != PackageKit::Transaction::ExitSuccess) {
            d->endWorkingOn("TOR is not available, or has an unexpected name. Please install it manually.", true);
            qWarning() << "resolve failed" << status;
            delete pkgs;
            resolveTransaction->deleteLater();
            return;
        }
        QStringList pkgids = pkgs->values();
        delete pkgs;

        if (pkgids.isEmpty()) {
            d->endWorkingOn("There was nothing new to install.");
            qDebug() << "Nothing to install";
        } else {
            qDebug() << "installing..." << pkgids;
            pkgids.removeDuplicates();
            auto installTransaction = PackageKit::Daemon::global()->installPackages(pkgids);
            QObject::connect(installTransaction, &PackageKit::Transaction::percentageChanged, qApp, [this, installTransaction]() {
                if(installTransaction->percentage() < 101) {
                    d->workingOn = i18n("Installing TOR (%1%)").arg(QString::number(installTransaction->percentage()));
                }
                else {
                    d->workingOn = i18n("Installing TOR...");
                }
                emit workingOnChanged();
            });
            QObject::connect(installTransaction, &PackageKit::Transaction::finished, qApp, [this, installTransaction](PackageKit::Transaction::Exit status) {
                if(status == PackageKit::Transaction::ExitSuccess) {
                    d->endWorkingOn("Installed TOR");
                }
                else {
                    d->endWorkingOn("TOR installation failed!", true);
                }
                installTransaction->deleteLater();
            });
        }
        resolveTransaction->deleteLater();
    });
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(torcontrol, torcontrol, "metadata.json")

#include "torcontrol.moc"
