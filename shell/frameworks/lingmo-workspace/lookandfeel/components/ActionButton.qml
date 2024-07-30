/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2024 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

LingmoComponents3.AbstractButton {
    id: root
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    font.pointSize: LingmoUI.Theme.defaultFont.pointSize + 1
    font.underline: root.activeFocus

    icon.width: LingmoUI.Units.iconSizes.large
    icon.height: LingmoUI.Units.iconSizes.large

    hoverEnabled: true

    // Expand clickable area, keep background centered
    leftInset: Math.max(LingmoUI.Units.largeSpacing * 2, (implicitContentWidth - implicitBackgroundWidth) / 2)
    rightInset: leftInset

    padding: LingmoUI.Units.smallSpacing
    // Labels wider than the background shouldn't be padded
    horizontalPadding: 0
    // No padding below label
    bottomPadding: 0

    // padding for circle and spacing between circle and label
    spacing: padding + LingmoUI.Units.smallSpacing

    opacity: root.activeFocus || root.hovered ? 1 : 0.85
    Behavior on opacity {
        PropertyAnimation { // OpacityAnimator makes it turn black at random intervals
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    background: Rectangle {
        implicitWidth: root.icon.width + root.padding * 2
        implicitHeight: root.icon.height + root.padding * 2
        // explicitly set size to keep it from expanding or shrinking
        width: implicitWidth
        height: implicitHeight
        radius: width / 2
        color: root.softwareRendering ? LingmoUI.Theme.backgroundColor : LingmoUI.Theme.textColor
        opacity: {
            if (root.activeFocus || root.hovered) {
                return root.softwareRendering ? 0.8 : 0.15
            }
            return root.softwareRendering ? 0.6 : 0
        }
        Behavior on opacity {
            PropertyAnimation { // OpacityAnimator makes it turn black at random intervals
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: LingmoUI.Theme.textColor
            opacity: 0.15
            scale: root.down ? 1 : 0
            Behavior on scale {
                PropertyAnimation {
                    duration: LingmoUI.Units.shortDuration
                    easing.type: Easing.InOutQuart
                }
            }
        }
    }

    contentItem: Column {
        spacing: root.spacing
        LingmoUI.Icon {
            anchors.horizontalCenter: parent.horizontalCenter
            source: root.icon.name
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            active: root.hovered || root.activeFocus
        }
        LingmoComponents3.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, parent.width)
            text: root.text
            style: root.softwareRendering ? Text.Outline : Text.Normal
            styleColor: LingmoUI.Theme.backgroundColor // Unused without outline
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
        }
    }

    Keys.onEnterPressed: clicked()
    Keys.onReturnPressed: clicked()
}
