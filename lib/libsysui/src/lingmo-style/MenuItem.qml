import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12

import LingmoUI 1.0 as LingmoUI

T.MenuItem
{
    id: control

    property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.2)
                                                       : Qt.rgba(0, 0, 0, 0.1)
    property color pressedColor: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                       : Qt.rgba(0, 0, 0, 0.2)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    verticalPadding: LingmoUI.Units.smallSpacing
    hoverEnabled: true
    topPadding: LingmoUI.Units.smallSpacing
    bottomPadding: LingmoUI.Units.smallSpacing

    icon.width: LingmoUI.Units.iconSizes.medium
    icon.height: LingmoUI.Units.iconSizes.medium

    icon.color: control.enabled ? (control.highlighted ? control.LingmoUI.Theme.highlightColor : control.LingmoUI.Theme.textColor) :
                             control.LingmoUI.Theme.disabledTextColor

    contentItem: IconLabel {
        readonly property real arrowPadding: control.subMenu && control.arrow ? control.arrow.width + control.spacing : 0
        readonly property real indicatorPadding: control.checkable && control.indicator ? control.indicator.width + control.spacing : 0
        leftPadding: !control.mirrored ? indicatorPadding + LingmoUI.Units.smallSpacing * 2 : arrowPadding
        rightPadding: control.mirrored ? indicatorPadding : arrowPadding + LingmoUI.Units.smallSpacing * 2

        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: Qt.AlignLeft

        icon: control.icon
        text: control.text
        font: control.font
        color: control.enabled ? control.pressed || control.hovered ? control.LingmoUI.Theme.textColor : 
               LingmoUI.Theme.textColor : control.LingmoUI.Theme.disabledTextColor
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: control.visible ? LingmoUI.Units.gridUnit + LingmoUI.Units.largeSpacing : 0
        radius: LingmoUI.Theme.mediumRadius
        opacity: 1

        anchors {
            fill: parent
            leftMargin: LingmoUI.Units.smallSpacing
            rightMargin: LingmoUI.Units.smallSpacing
        }

        color: control.pressed || highlighted ? control.pressedColor : control.hovered ? control.hoveredColor : "transparent"
    }
}
