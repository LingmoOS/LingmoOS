import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T
import LingmoUI

T.Frame {
    id: control
    property alias border: d.border
    property alias color: d.color
    property alias radius: d.radius
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, contentHeight + topPadding + bottomPadding)
    padding: 0
    background: Rectangle {
        id: d
        radius: LingmoUnits.mediumRadius
        border.color: LingmoTheme.dividerColor
        color: {
            if (Window.active) {
                return LingmoTheme.frameActiveColor;
            }
            return LingmoTheme.frameColor;
        }
    }
}
