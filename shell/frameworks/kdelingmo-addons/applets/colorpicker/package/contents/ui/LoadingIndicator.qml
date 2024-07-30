/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3

/**
 * Visible when an image is being processed by \LingmoUI.ImageColors
 */
LingmoComponents3.BusyIndicator {
    id: loadingIndicator
    anchors.fill: parent

    property int jobRemaining: 0

    signal jobDone

    onJobDone: {
        jobRemaining -= 1;
        if (!jobRemaining) {
            animator.start();
        }
    }

    onJobRemainingChanged: selfDestructionTimer.restart();

    OpacityAnimator {
        id: animator
        easing.type: Easing.InQuad
        from: 1
        to: 0
        target: loadingIndicator
        duration: LingmoUI.Units.longDuration

        onStopped: selfDestructionTimer.triggered();
    }

    Timer {
        id: selfDestructionTimer
        interval: 60000
        running: true

        onTriggered: {
            loadingIndicator.parent.loadingIndicator = null;
            loadingIndicator.destroy();
        }
    }
}
