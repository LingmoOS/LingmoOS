import QtQuick 2.4
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI

Item {
    id: control
    height: LingmoUI.Units.smallSpacing

    Layout.fillWidth: true

    Rectangle {
        anchors.centerIn: parent
        height: 1
        width: control.width
        color: LingmoUI.Theme.disabledTextColor
        opacity: LingmoUI.Theme.darkMode ? 0.4 : 0.4
    }
}
