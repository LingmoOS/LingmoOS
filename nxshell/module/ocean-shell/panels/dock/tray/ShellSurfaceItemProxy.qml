// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor
import org.lingmo.ds.dock 1.0

Item {
    id: root
    property var shellSurface: null
    signal surfaceDestroyed()
    property bool autoClose: false
    property bool inputEventsEnabled: true
    property bool hovered: hoverHandler.hovered
    property bool pressed: tapHandler.pressed

    implicitWidth: shellSurface.width
    implicitHeight: shellSurface.height

    ShellSurfaceItem {
        id: impl
        anchors.fill: parent
        shellSurface: root.shellSurface
        inputEventsEnabled: root.inputEventsEnabled

        HoverHandler {
            id: hoverHandler
        }
        TapHandler {
            id: tapHandler
        }

        onVisibleChanged: function () {
            if (autoClose && !visible) {
                // surface is valid but client's shellSurface maybe invalid.
                Qt.callLater(closeShellSurface)
            }
        }
        function closeShellSurface()
        {
            if (surface && shellSurface) {
                DockCompositor.closeShellSurface(shellSurface)
            }
        }
    }
    Component.onCompleted: function () {
        impl.surfaceDestroyed.connect(root.surfaceDestroyed)
    }

    Connections {
        target: shellSurface
        // TODO it's maybe a bug for qt, we force shellSurface's value to update
        function onAboutToDestroy()
        {
            Qt.callLater(function() {
                impl.shellSurface = null
                impl.shellSurface = Qt.binding(function () {
                    return root.shellSurface
                })
            })
        }
    }
}
