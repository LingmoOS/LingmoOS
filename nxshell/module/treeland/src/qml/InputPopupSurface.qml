// Copyright (C) 2023 Yixue Wang <wangyixue@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Waylib.Server

InputPopupSurfaceItem {

    property var cursorPoint: Qt.point(referenceRect.x, referenceRect.y)
    property real cursorWidth: referenceRect.width
    property real cursorHeight: referenceRect.height

    x: {
        var x = cursorPoint.x + cursorWidth
        if (x + width > parent.width) {
            x = Math.max(0, parent.width - width)
        }
        return x
    }

    y: {
        var y = cursorPoint.y + cursorHeight
        if (y + height > parent.height) {
            y = Math.min(cursorPoint.y - height, parent.height - height)
        }
        return y
    }

    OutputLayoutItem {
        anchors.fill: parent
        layout: Helper.outputLayout

        onEnterOutput: function(output) {
            surface.surface.enterOutput(output);
        }
        onLeaveOutput: function(output) {
            surface.surface.leaveOutput(output);
        }
    }
}
