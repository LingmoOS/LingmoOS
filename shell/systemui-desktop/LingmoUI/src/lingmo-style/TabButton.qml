import QtQuick 2.9
import QtQuick.Templates 2.2 as T
import LingmoUI 1.0 as LingmoUI

T.TabButton {
    id: control

    property int standardHeight: LingmoUI.Units.iconSizes.medium + LingmoUI.Units.smallSpacing
    property color pressedColor: Qt.rgba(LingmoUI.Theme.textColor.r, LingmoUI.Theme.textColor.g, LingmoUI.Theme.textColor.b, 0.5)

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             standardHeight)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    padding: 0
    spacing: 0

    contentItem: Text {
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        text: control.text
        font: control.font
        color: !control.enabled ? LingmoUI.Theme.disabledTextColor : control.pressed ? pressedColor : control.checked ? LingmoUI.Theme.textColor : LingmoUI.Theme.textColor
    }
}
