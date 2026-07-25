import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import LingmoUI

LingmoFilledButton {
    id: control
    display: Button.IconOnly
    property int iconSize: 20
    property int iconSource
    property bool disabled: false
    property int radius: LingmoUnits.smallRadiuss
    property string contentDescription: ""
    property Component iconDelegate: com_icon

    property color iconColor: {
        if (!enabled) {
            return Qt.rgba(130 / 255, 130 / 255, 130 / 255, 1);
        }
        return Qt.rgba(1, 1, 1, 1);
    }

    Accessible.role: Accessible.Button
    Accessible.name: control.text
    Accessible.description: contentDescription
    Accessible.onPressAction: control.clicked()
    focusPolicy: Qt.TabFocus
    padding: 0
    verticalPadding: 8
    horizontalPadding: 8
    enabled: !disabled
    font: LingmoTextStyle.Caption

    background: LingmoControlBackground {
        implicitWidth: 30
        implicitHeight: 30
        radius: control.radius
        bottomMargin: enabled ? 2 : 0
        border.width: enabled ? 1 : 0
        border.color: enabled ? Qt.darker(control.normalColor, 1.2) : disableColor
        color: {
            if (!enabled) {
                return disableColor;
            }
            if (pressed) {
                return pressedColor;
            }
            return hovered ? hoverColor : normalColor;
        }
        LingmoFocusRectangle {
            visible: control.visualFocus
            radius: 4
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
                return com_column;
            }
            return com_row;
        }
    }
    LingmoTooltip {
        id: tool_tip
        visible: {
            if (control.text === "") {
                return false;
            }
            if (control.display !== Button.IconOnly) {
                return false;
            }
            return hovered;
        }
        text: control.text
        delay: 1000
    }
}
