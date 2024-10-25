/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmoui as LingmoUI

SequentialAnimation {
    id: pressedAnimation
    objectName: "pressedAnimation"

    property Item targetItem
    property int duration: LingmoUI.Units.shortDuration

    // Fast scaling while we're animation == more FPS
    ScriptAction { script: targetItem.smooth = false }

    ParallelAnimation {
        PropertyAnimation {
            target: targetItem
            properties: "opacity"
            from: 1.0; to: 0.8
            duration: pressedAnimation.duration;
            easing.type: Easing.OutExpo;
        }
        PropertyAnimation {
            target: targetItem
            properties: "scale"
            from: 1.0; to: 0.95
            duration: pressedAnimation.duration;
            easing.type: Easing.OutExpo;
        }
    }
    ScriptAction { script: targetItem.smooth = true }
}
