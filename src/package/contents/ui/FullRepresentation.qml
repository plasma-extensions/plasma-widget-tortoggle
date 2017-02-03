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
        enabled: opacity > 0;
        opacity: plasmoid.nativeInterface.status == 3 ? 1 : 0;
        Behavior on opacity { NumberAnimation { duration: units.longDuration; } }
        anchors {
            fill: parent;
            margins: units.smallSpacing;
        }
        PlasmaCore.IconItem {
            anchors {
                top: parent.verticalCenter;
                right: parent.right;
                bottom: parent.bottom;
            }
            width: height;
            source: "dialog-warning";
            opacity: 0.3;
        }
        PlasmaComponents.Label {
            anchors.fill: parent;
            wrapMode: Text.WordWrap;
            verticalAlignment: Text.AlignTop;
            text: i18n("You do not have TOR installed on this system. Please install this, either by <a href=\"installTOR\">clicking here</a>, or install it manually (making sure it is in your path).");
            onLinkActivated: {
                if(link == "installTOR") {
                    plasmoid.nativeInterface.installTOR();
                }
            }
            MouseArea {
                anchors.fill: parent;
                cursorShape: parent.hoveredLink != "" ? Qt.PointingHandCursor : Qt.ArrowCursor;
                acceptedButtons: Qt.NoButton;
            }
        }
    }

    Item {
        enabled: opacity > 0;
        opacity: plasmoid.nativeInterface.status < 3 ? 1 : 0;
        Behavior on opacity { NumberAnimation { duration: units.longDuration; } }
        anchors.fill: parent;
        Item {
            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
                bottom: parent.verticalCenter;
                margins: units.smallSpacing;
            }
            PlasmaCore.IconItem {
                id: icon;
                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: parent.left;
                    margins: units.smallSpacing;
                }
                source: "torcontrol";
            }
            PlasmaComponents.Label {
                anchors {
                    top: parent.top;
                    left: icon.right;
                    right: parent.right;
                    bottom: parent.bottom;
                }
                verticalAlignment: Text.AlignVCenter;
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

    Rectangle {
        color: PlasmaCore.ColorScope.backgroundColor;
        enabled: opacity > 0;
        opacity: plasmoid.nativeInterface.workingOn !== "" ? 1 : 0;
        Behavior on opacity { NumberAnimation { duration: units.longDuration; } }
        anchors.fill: parent;
        MouseArea {
            anchors.fill: parent;
            onClicked: {}
        }
        PlasmaComponents.BusyIndicator {
            anchors {
                horizontalCenter: parent.horizontalCenter;
                bottom: parent.verticalCenter;
                margins: units.largeSpacing;
            }
            running: parent.enabled;
        }
        PlasmaComponents.Label {
            anchors {
                top: parent.verticalCenter;
                left: parent.left;
                right: parent.right;
                bottom: parent.bottom;
            }
            wrapMode: Text.WordWrap;
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignTop;
            text: plasmoid.nativeInterface.workingOn;
        }
    }
}
