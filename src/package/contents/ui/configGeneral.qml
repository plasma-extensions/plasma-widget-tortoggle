import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: configPage;
    width: childrenRect.width;
    height: childrenRect.height;
    implicitWidth: mainColumn.implicitWidth;
    implicitHeight: pageColumn.implicitHeight;

    property alias cfg_systemTor: systemTorField.checked;

    Column {
        id: pageColumn;
        anchors.fill: parent;
        spacing: 4;
        Row {
            QtControls.Label {
                text: "Use system tor instance (requires super user access, aka sudo)";
            }
            QtControls.CheckBox {
                id: systemTorField;
            }
        }
    }
}
