import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import LingmoUI 1.0 as LingmoUI

T.MenuSeparator {
    id: control

    implicitHeight: LingmoUI.Units.largeSpacing + separator.height
    width: parent.width

    background: Rectangle {
        id: separator
        anchors.centerIn: control
        width: control.width - LingmoUI.Units.largeSpacing * 2
        height: 1
        color: LingmoUI.Theme.textColor
        opacity: 0.3
    }
}