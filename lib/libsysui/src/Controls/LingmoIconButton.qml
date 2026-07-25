import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import LingmoUI

Button {
    display: Button.IconOnly
    property int iconSize: 20
    property int iconSource
    property bool disabled: false
    property int radius: LingmoUnits.smallRadius
    property string contentDescription: ""
    property color hoverColor: LingmoTheme.itemHoverColor
    property color pressedColor: LingmoTheme.itemPressColor
    property color normalColor: LingmoTheme.itemNormalColor
    property color disableColor: LingmoTheme.itemNormalColor
    property Component iconDelegate: com_icon
    property color color: {
        if (!enabled) {
            return disableColor
        }
        if (pressed) {
            return pressedColor
        }
        return hovered ? hoverColor : normalColor
    }
    property color iconColor: {
        if (LingmoTheme.dark) {
            if (!enabled) {
                return Qt.rgba(130 / 255, 130 / 255, 130 / 255, 1)
            }
            return Qt.rgba(1, 1, 1, 1)
        } else {
            if (!enabled) {
                return Qt.rgba(161 / 255, 161 / 255, 161 / 255, 1)
            }
            return Qt.rgba(0, 0, 0, 1)
        }
    }
    property color textColor: LingmoTheme.fontPrimaryColor
    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: contentDescription
    Accessible.onPressAction: control.clicked()
    id: control
    focusPolicy: Qt.TabFocus
    padding: 0
    verticalPadding: 8
    horizontalPadding: 8
    enabled: !disabled
    font: LingmoTextStyle.Caption
    background: Rectangle {
        implicitWidth: 30
        implicitHeight: 30
        radius: control.radius
        color: control.color
        LingmoFocusRectangle {
            visible: control.activeFocus
        }
    }
    Component {
        id: com_icon
        LingmoIcon {
            id: text_icon
            font.pixelSize: iconSize
            iconSize: control.iconSize
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            iconColor: control.iconColor
            iconSource: control.iconSource
        }
    }
    Component {
        id: com_row
        RowLayout {
            LingmoLoader {
                sourceComponent: iconDelegate
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: display !== Button.TextOnly
            }
            LingmoText {
                text: control.text
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: display !== Button.IconOnly
                color: control.textColor
                font: control.font
            }
        }
    }
    Component {
        id: com_column
        ColumnLayout {
            LingmoLoader {
                sourceComponent: iconDelegate
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: display !== Button.TextOnly
            }
            LingmoText {
                text: control.text
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                visible: display !== Button.IconOnly
                color: control.textColor
                font: control.font
            }
        }
    }
    contentItem: LingmoLoader {
        sourceComponent: {
            if (display === Button.TextUnderIcon) {
                return com_column
            }
            return com_row
        }
    }
    LingmoTooltip {
        id: tool_tip
        visible: {
            if (control.text === "") {
                return false
            }
            if (control.display !== Button.IconOnly) {
                return false
            }
            return hovered
        }
        text: control.text
        delay: 1000
    }
}
