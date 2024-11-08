/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.ksvg as KSvg
import org.kde.lingmoui as LingmoUI

Item {
    id: root
    required property bool hovered
    required property bool pressed
    required property bool checked
    required property bool focused

    readonly property bool showPressed: checked || pressed
    readonly property bool showFocused: focused && surfaceFocused.usedPrefix != "normal"

    property var margins: showPressed ? surfacePressed.margins : (showFocused ? surfaceFocused.margins : surfaceNormal.margins)
    property real leftMargin: margins.left
    property real topMargin: margins.top
    property real rightMargin: margins.right
    property real bottomMargin: margins.bottom
    property string usedPrefix: showPressed ? surfacePressed.usedPrefix : (showFocused ? surfaceFocused.usedPrefix : surfaceNormal.usedPrefix)

    ButtonShadow {
        anchors.fill: parent
        showShadow: enabled && !root.checked && !root.pressed
    }

    KSvg.FrameSvgItem {
        id: surfaceNormal
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix:  "normal"
    }

    // Intentionally lower than surfacePressed, surfaceFocused
    ButtonFocus {
        anchors.fill: parent
        showFocus: root.focused && !root.pressed
    }

    // Intentionally lower than surfacePressed and surfaceFocused
    ButtonHover {
        anchors.fill: parent
        showHover: root.hovered && !root.pressed
    }

    KSvg.FrameSvgItem {
        id: surfacePressed
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "pressed"
        opacity: root.showPressed ? 1 : 0
        Behavior on opacity {
            enabled: LingmoUI.Units.shortDuration > 0
            OpacityAnimator { duration: LingmoUI.Units.shortDuration; easing.type: Easing.OutQuad }
        }
    }

    KSvg.FrameSvgItem {
        id: surfaceFocused
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: ["focus-background", "normal"]
        opacity: root.showFocused ? 1 : 0
        Behavior on opacity {
            enabled: LingmoUI.Units.shortDuration > 0
            OpacityAnimator { duration: LingmoUI.Units.shortDuration; easing.type: Easing.OutQuad }
        }
    }

}
