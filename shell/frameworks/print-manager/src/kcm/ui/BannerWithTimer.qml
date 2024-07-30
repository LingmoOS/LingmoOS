/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.InlineMessage {
    id: banner
    type: LingmoUI.MessageType.Error
    position: LingmoUI.InlineMessage.Position.Header
    showCloseButton: true

    property bool resetToDefault: true
    property int defaultType: LingmoUI.MessageType.Error
    property alias autoCloseInterval: timer.interval

    signal timeout()

    onVisibleChanged: {
        if (visible && timer.interval > 0) {
            timer.start()
        }
    }

    onResetToDefaultChanged: {
        if (resetToDefault) {
            reset()
        }
    }

    function reset() {
        timer.stop()
        timer.interval = 10000
        visible = false
        type = defaultType
    }

    Timer {
        id: timer
        repeat: false; interval: 10000
        onTriggered: {
            banner.visible = false
            if (banner.resetToDefault) {
                reset()
            }
            timeout()
        }
    }
}

