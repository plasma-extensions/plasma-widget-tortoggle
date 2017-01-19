/***************************************************************************
 *   Copyright (C) 2017 by Dan Leinir Turthra Jensen <admin@leinir.dk>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    Layout.minimumWidth: Layout.minimumHeight * 1.333
    Layout.minimumHeight: units.gridUnit * 10
    Layout.preferredWidth: Layout.minimumWidth * 1.5
    Layout.preferredHeight: Layout.minimumHeight * 1.5

    Item {
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
            bottom: parent.verticalCenter;
            margins: units.smallSpacing;
        }
        PlasmaComponents.Label {
            anchors.fill: parent;
            text: i18n("Switch your instance of The Onion Router on and off.");
            wrapMode: Text.WordWrap;
        }
    }
    Item {
        anchors {
            top: parent.verticalCenter;
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
            margins: units.smallSpacing;
        }
        PlasmaComponents.Label {
            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
                bottom: parent.verticalCenter;
                margins: units.smallSpacing;
            }
            verticalAlignment: Text.AlignBottom;
            horizontalAlignment: Text.AlignHCenter;
            text: {
                switch(plasmoid.nativeInterface.status) {
                    case 1:
                        return i18n("TOR is running");
                        break;
                    case 2:
                        return i18n("TOR is not running");
                        break;
                    case 0:
                    default:
                        return i18n("TOR status is unknown");
                        break;
                }
            }
        }
        PlasmaComponents.Button {
            id: statusIcon;
            anchors {
                top: parent.verticalCenter;
                horizontalCenter: parent.horizontalCenter;
                margins: units.smallSpacing;
            }
            text: plasmoid.nativeInterface.buttonLabel;
            iconName: plasmoid.nativeInterface.iconName;
            tooltip: plasmoid.nativeInterface.buttonLabel;
            onClicked: root.changeRunningStatus();
        }
    }
}
