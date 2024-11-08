/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmoui as LingmoUI

SequentialAnimation {
    id: releasedAnimation
    objectName: "releasedAnimation"

    property Item targetItem
    property int duration: LingmoUI.Units.shortDuration

    // Fast scaling while we're animation == more FPS
    ScriptAction { script: targetItem.smooth = false }

    ParallelAnimation {
        PropertyAnimation {
            target: targetItem
            properties: "opacity"
            from: 0.8; to: 1.0
            duration: releasedAnimation.duration;
            easing.type: Easing.InExpo;
        }
        PropertyAnimation {
            target: targetItem
            properties: "scale"
            from: 0.95; to: 1.0
            duration: releasedAnimation.duration;
            easing.type: Easing.InExpo;
        }
    }

    ScriptAction { script: targetItem.smooth = true }
}
