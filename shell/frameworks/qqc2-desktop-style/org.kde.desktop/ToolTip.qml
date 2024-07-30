/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.ToolTip {
    id: control

    LingmoUI.Theme.colorSet: parent && parent.LingmoUI.Theme.colorSet === LingmoUI.Theme.Complementary
        ? LingmoUI.Theme.Complementary : LingmoUI.Theme.Tooltip
    LingmoUI.Theme.inherit: false

    x: parent ? Math.round((parent.width - implicitWidth) / 2) : 0
    y: -implicitHeight - 3

    z: LingmoUI.OverlayZStacking.z

    // Math.ceil() prevents blurry edges and prevents unnecessary text wrapping
    // (vs using floor or sometimes round).
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            Math.ceil(contentWidth) + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             Math.ceil(contentHeight) + topPadding + bottomPadding)

    margins: 6
    padding: 6

    visible: {
        if (!parent || text.length === 0) {
            return false;
        }
        if (LingmoUI.Settings.tabletMode) {
            return parent.pressed ?? false;
        }
        return parent.hovered ?? parent.containsMouse ?? false;
    }
    delay: LingmoUI.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : LingmoUI.Units.toolTipDelay
    // Never time out while being hovered; it's annoying
    timeout: -1

    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    contentItem: Item {
        implicitWidth: Math.min(label.maxTextLength, label.contentWidth)
        implicitHeight: label.implicitHeight

        Label {
            id: label

            // This value is basically arbitrary. It just looks nice.
            readonly property double maxTextLength: LingmoUI.Units.gridUnit * 14

            // Strip out ampersands right before non-whitespace characters, i.e.
            // those used to determine the alt key shortcut
            // (except when the word ends in ; (HTML entities))
            text: control.text.replace(/(&)(?!;)\S+(?>\s)/g, "")
            // Using Wrap instead of WordWrap to prevent tooltips with long URLs
            // from overflowing
            wrapMode: Text.Wrap
            font: control.font
            color: LingmoUI.Theme.textColor

            LingmoUI.Theme.colorSet: control.LingmoUI.Theme.colorSet
            // ensure that long text actually gets wrapped
            onLineLaidOut: (line) => {
                if (line.implicitWidth > maxTextLength)
                    line.width = maxTextLength
            }
        }
    }

    // TODO: Consider replacing this with a StyleItem
    background: LingmoUI.ShadowedRectangle {
        radius: LingmoUI.Units.cornerRadius
        color: LingmoUI.Theme.backgroundColor
        LingmoUI.Theme.colorSet: control.LingmoUI.Theme.colorSet

        // Roughly but doesn't exactly match the medium shadow setting for Ocean menus/tooltips.
        // TODO: Find a way to more closely match the user's Ocean settings.
        shadow.xOffset: 0
        shadow.yOffset: 4
        shadow.size: 16
        shadow.color: Qt.rgba(0, 0, 0, 0.2)

        border.width: 1
        // TODO: Replace this with a frame or separator color role if that becomes a thing.
        // Matches the color used by Ocean::Style::drawPanelTipLabelPrimitive()
        border.color: LingmoUI.ColorUtils.linearInterpolation(color, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast)
    }
}
