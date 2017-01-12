import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0 as QtLayouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: configPage;
    width: childrenRect.width;
    height: childrenRect.height;
    implicitWidth: mainColumn.implicitWidth;
    implicitHeight: pageColumn.implicitHeight;

    property alias cfg_systemTor: systemTorField.checked;

    QtLayouts.ColumnLayout {
        id: pageColumn;
        anchors.left: parent.left
        QtControls.CheckBox {
            id: systemTorField;
            text: "Use system tor instance (requires super user access, aka sudo)";
        }
    }
}
