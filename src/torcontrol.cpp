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
    void updateStatus() {
        if(systemTor && whatInit == SystemDInit) {
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
                // We've just started up, find out whether we've actually got a running tor instance or not...
                findTorPid();
            }
            else if(torPid == -2) {
                // We've already been stopped, or we checked, so we know our status explicitly
                status = NotRunning;
            }
        }
        emit q->statusChanged();
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

K_EXPORT_PLASMA_APPLET_WITH_JSON(torcontrol, torcontrol, "metadata.json")

#include "torcontrol.moc"
