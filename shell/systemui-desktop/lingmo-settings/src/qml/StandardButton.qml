import QtQuick 2.12
import QtQuick.Controls 2.12
import LingmoUI 1.0 as LingmoUI

Button {
    id: control

    property color backgroundColor: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
    property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(backgroundColor, 1.3)
                                                       : Qt.darker(backgroundColor, 1.1)
    property color pressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(backgroundColor, 1.1)
                                                       : Qt.darker(backgroundColor, 1.2)

    scale: 1

    background: Rectangle {
        radius: LingmoUI.Theme.mediumRadius
        color: control.pressed ? control.pressedColor : control.hovered ? control.hoveredColor : control.backgroundColor
    }
}
