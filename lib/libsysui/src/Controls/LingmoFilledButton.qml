import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import LingmoUI

Button {
    property bool disabled: false
    property string contentDescription: ""
    property color normalColor: LingmoTheme.primaryColor
    property color hoverColor: LingmoTheme.dark ? Qt.darker(normalColor,
                                                            1.1) : Qt.lighter(
                                                      normalColor, 1.1)
    property color disableColor: LingmoTheme.dark ? Qt.rgba(
                                                        82 / 255, 82 / 255,
                                                        82 / 255,
                                                        1) : Qt.rgba(199 / 255, 199
                                                                     / 255, 199 / 255, 1)
    property color pressedColor: LingmoTheme.dark ? Qt.darker(normalColor,
                                                              1.2) : Qt.lighter(
                                                        normalColor, 1.2)
    property color textColor: {
        if (LingmoTheme.dark) {
            if (!enabled) {
                return Qt.rgba(173 / 255, 173 / 255, 173 / 255, 1)
            }
            return Qt.rgba(0, 0, 0, 1)
        } else {
            return Qt.rgba(1, 1, 1, 1)
        }
    }
    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: contentDescription
    Accessible.onPressAction: control.clicked()
    id: control
    enabled: !disabled
    focusPolicy: Qt.TabFocus
    font: LingmoTextStyle.Body
    verticalPadding: 0
    horizontalPadding: 12
    background: LingmoControlBackground {
        implicitWidth: 30
        implicitHeight: 30
        radius: LingmoUnits.windowRadius
        bottomMargin: enabled ? 2 : 0
        border.width: enabled ? 1 : 0
        border.color: enabled ? Qt.darker(control.normalColor,
                                          1.2) : disableColor
        color: {
            if (!enabled) {
                return disableColor
            }
            if (pressed) {
                return pressedColor
            }
            return hovered ? hoverColor : normalColor
        }
        LingmoFocusRectangle {
            visible: control.visualFocus
            radius: 4
        }
    }
    contentItem: LingmoText {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: control.textColor
    }
}
