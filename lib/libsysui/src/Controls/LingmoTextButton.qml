import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import LingmoUI

Button {
    property bool disabled: false
    property string contentDescription: ""
    property color normalColor: LingmoTheme.primaryColor
    property color hoverColor: LingmoTheme.dark ? Qt.darker(normalColor,
                                                         1.15) : Qt.lighter(
                                                   normalColor, 1.15)
    property color pressedColor: LingmoTheme.dark ? Qt.darker(normalColor,
                                                           1.3) : Qt.lighter(
                                                     normalColor, 1.3)
    property color disableColor: LingmoTheme.dark ? Qt.rgba(
                                                     82 / 255, 82 / 255,
                                                     82 / 255,
                                                     1) : Qt.rgba(199 / 255, 199
                                                                  / 255, 199 / 255, 1)
    property color backgroundHoverColor: LingmoTheme.itemHoverColor
    property color backgroundPressedColor: LingmoTheme.itemPressColor
    property color backgroundNormalColor: LingmoTheme.itemNormalColor
    property color backgroundDisableColor: LingmoTheme.itemNormalColor
    property bool textBold: true
    property color textColor: {
        if (!enabled) {
            return disableColor
        }
        if (pressed) {
            return pressedColor
        }
        return hovered ? hoverColor : normalColor
    }
    id: control
    horizontalPadding: 6
    enabled: !disabled
    font: LingmoTextStyle.Body
    background: Rectangle {
        implicitWidth: 30
        implicitHeight: 30
        radius: LingmoUnits.windowRadius
        color: {
            if (!enabled) {
                return backgroundDisableColor
            }
            if (pressed) {
                return backgroundPressedColor
            }
            if (hovered) {
                return backgroundHoverColor
            }
            return backgroundNormalColor
        }
        LingmoFocusRectangle {
            visible: control.visualFocus
            radius: 8
        }
    }
    focusPolicy: Qt.TabFocus
    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: contentDescription
    Accessible.onPressAction: control.clicked()
    contentItem: LingmoText {
        id: btn_text
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: control.textColor
    }
}
