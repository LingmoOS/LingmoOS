// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    required property SurfaceItem surface
    required property ToplevelSurface waylandSurface
    required property DynamicCreatorComponent creator

    property OutputItem output
    property CoordMapper outputCoordMapper
    property bool mapped: waylandSurface.surface && waylandSurface.surface.mapped && waylandSurface.WaylandSocket.rootSocket.enabled
    property bool pendingDestroy: false

    OpacityAnimator {
        id: hideAnimation
        duration: 300
        target: surface
        from: 1
        to: 0

        onStopped: {
            surface.visible = false
            if (pendingDestroy)
                creator.destroyObject(surface)
        }
    }

    Connections {
        target: surface

        function onEffectiveVisibleChanged() {
            if (surface.effectiveVisible) {
                // Apply the WSurfaceItem's size to wl_surface
                surface.resize(SurfaceItem.SizeToSurface)
                surface.resizeMode = SurfaceItem.SizeToSurface

                if (waylandSurface && waylandSurface.isActivated)
                    surface.forceActiveFocus()
            } else {
                surface.resizeMode = SurfaceItem.ManualResize
            }
        }
    }

    onMappedChanged: {
        if (pendingDestroy)
            return

        if (mapped && surface.effectiveVisible)
            Helper.activatedSurface = waylandSurface

        // When Socket is enabled and mapped becomes false, set visible
        // after hideAnimation complete， Otherwise set visible directly.
        if (mapped || !waylandSurface.WaylandSocket.rootSocket.enabled) {
            surface.visible = mapped
            return
        }

        // do animation for window close
        hideAnimation.start()
    }

    function doDestroy() {
        pendingDestroy = true

        if (!surface.visible || !hideAnimation.running) {
            creator.destroyObject(surface)
            return
        }

        // unbind some properties
        mapped = visible
    }

    Connections {
        target: waylandSurface

        function onActivateChanged() {
            if (waylandSurface.isActivated) {
                if (surface.effectiveVisible)
                    surface.forceActiveFocus()
            } else {
                surface.focus = false
            }
        }
    }
}
