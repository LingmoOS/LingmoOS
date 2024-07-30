/*
 *   SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton

import QtQuick

Item {
    property color color: _randomColor()

    // Intentionally not using standard durations because this is an
    // animated wallpaper - animations shouldn't be disabled and the
    // duration shouldn't be scaled with the user's preferences.
    readonly property int fastDuration: 2000
    readonly property int slowDuration: 30000

    // For accent color, tell the wallpaper we've changed the color
    signal repaintNeeded()

    Behavior on color {
        SequentialAnimation {
            ColorAnimation {
                id: colorAnimation
                duration: slowDuration
                easing.type: Easing.InOutQuad
            }

            ScriptAction {
                script: updateColor()
            }
        }
    }

    function _randomColor() {
        return Qt.hsla(Math.random(), 1, 0.5, 1)
    }

    function updateColor(fast = false) {
        colorAnimation.duration = fast ? fastDuration : slowDuration;
        color = _randomColor();
        repaintNeeded();
    }

    Component.onCompleted: updateColor()
}
