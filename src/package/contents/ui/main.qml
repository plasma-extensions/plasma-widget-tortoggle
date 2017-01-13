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
    id: root;

    Plasmoid.icon: plasmoid.nativeInterface.iconName;
    Plasmoid.toolTipMainText: i18n("TOR Controller")
    Plasmoid.toolTipSubText: plasmoid.nativeInterface.buttonLabel

    function changeRunningStatus() {
        if(plasmoid.nativeInterface.status == 1) {
            plasmoid.nativeInterface.status = 2;
        }
        else {
            plasmoid.nativeInterface.status = 1;
        }
    }
    Binding {
        target: plasmoid.nativeInterface;
        property: "systemTor";
        value: plasmoid.configuration.systemTor;
    }

    Plasmoid.compactRepresentation: Item {
        PlasmaCore.IconItem {
            anchors {
                fill: parent;
                margins: units.smallSpacing;
            }
            source: "system-run";
            enabled: plasmoid.nativeInterface.status == 1;
            active: compactMouse.containsMouse;
        }
        MouseArea {
            id: compactMouse;
            anchors.fill: parent;
            hoverEnabled: true;
            onClicked: plasmoid.expanded = !plasmoid.expanded;
        }
    }
    Plasmoid.fullRepresentation: FullRepresentation { }
}
