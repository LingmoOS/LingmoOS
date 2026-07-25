import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T
import LingmoUI

T.ToolTip {
    id: control
    x: parent ? (parent.width - implicitWidth) / 2 : 0
    y: -implicitHeight - 3
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)
    margins: 6
    padding: 6
    font: LingmoTextStyle.Body
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent
                 | T.Popup.CloseOnReleaseOutsideParent
    contentItem: LingmoText {
        text: control.text
        font: control.font
        wrapMode: Text.Wrap
    }
    background: Rectangle {
        color: LingmoTheme.dark ? Qt.rgba(50 / 255, 49 / 255, 48 / 255,
                                          1) : Qt.rgba(1, 1, 1, 1)
        radius: LingmoUnits.smallRadius
        LingmoShadow {
            radius: LingmoUnits.smallRadius
        }
    }
}
