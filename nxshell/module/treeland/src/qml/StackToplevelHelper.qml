// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Waylib.Server
import QtQuick.Particles
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    id: root

    required property SurfaceItem surface
    required property ToplevelSurface waylandSurface
    required property DynamicCreatorComponent creator
    required property SurfaceItemFactory surfaceWrapper
    property WindowDecoration decoration
    property var quickForeignToplevelManageMapper: ForeignToplevelV1.Attached(waylandSurface)

    property OutputItem output
    property CoordMapper outputCoordMapper
    property bool mapped: waylandSurface.surface && waylandSurface.surface.mapped && waylandSurface.WaylandSocket.rootSocket.enabled
    property bool pendingDestroy: false
    property bool isMaximize: waylandSurface && waylandSurface.isMaximized && outputCoordMapper
    property bool isFullScreen: waylandSurface && waylandSurface.isFullScreen && outputCoordMapper
    property bool showCloseAnimation: false
    property bool showNewAnimation: true
    onIsMaximizeChanged: console.log(`${surface} isMaximize changed, ${waylandSurface.isMaximized}, ${outputCoordMapper}`)

    // For Maximize
    property rect maximizeRect: outputCoordMapper ? Qt.rect(
        outputCoordMapper.x + Helper.getLeftExclusiveMargin(waylandSurface),
        outputCoordMapper.y + Helper.getTopExclusiveMargin(waylandSurface),
        outputCoordMapper.width - Helper.getLeftExclusiveMargin(waylandSurface) - Helper.getRightExclusiveMargin(waylandSurface),
        outputCoordMapper.height - Helper.getTopExclusiveMargin(waylandSurface) - Helper.getBottomExclusiveMargin(waylandSurface)
    ) : Qt.rect(0,0,0,0)

    // For Fullscreen
    property rect fullscreenRect: outputCoordMapper ? Qt.rect(
        outputCoordMapper.x,
        outputCoordMapper.y,
        outputCoordMapper.width,
        outputCoordMapper.height
    ) : Qt.rect(0,0,0,0)

    Loader {
        id: newAnimation
    }

    Component {
        id: newAnimationComponent

        NewAnimation {
            target: surface
        }
    }

    Loader {
        id: closeAnimation
    }

    Component {
        id: closeAnimationComponent

        CloseAnimation {
            onStopped: {
                if (pendingDestroy)
                    creator.destroyObject(surfaceWrapper)
            }
        }
    }

    Repeater {
        model: [ root.quickForeignToplevelManageMapper, root.decoration, ]

        Item {
            Connections {
                target: modelData
                ignoreUnknownSignals: true

                function onRequestMaximize(maximized) {
                    if (maximized) {
                        doMaximize()
                    } else {
                        cancelMaximize()
                    }
                }

                function onRequestMinimize(minimized = true) {
                    if (minimized) {
                        doMinimize()
                    } else {
                        cancelMinimize()
                    }
                }

                function onRequestActivate(activated) {
                    if (activated && waylandSurface.isMinimized) {
                        cancelMinimize()
                    }

                    surface.focus = activated
                    if (activated)
                        Helper.activatedSurface = waylandSurface
                }

                function onRequestFullscreen(fullscreen) {
                    if (fullscreen) {
                        doFullscreen();
                    } else {
                        cancelFullscreen();
                    }
                }

                function onRequestClose() {
                    //quickForeignToplevelManageMapper.onRequestActivate(false) // TODO: @rewine
                    Helper.closeSurface(waylandSurface.surface);
                }

                function onRectangleChanged(edges) {
                    // error
                    doResize(null, edges, null)
                }

                function onRequestMove() {
                    doMove(null, 0)
                }

                function onRequestResize(edges) {
                    doResize(null, edges, null)
                }
            }
        }
    }

    function workspace() { return QmlHelper.workspaceManager.workspacesById.get(parent.workspaceId) }

    function setAllSurfacesVisble(visible) {
        for (var i = 0; i < QmlHelper.workspaceManager.allSurfaces.count; ++i) {
            QmlHelper.workspaceManager.allSurfaces.get(i).item.visible = visible
        }
    }

    onMappedChanged: {
        console.log("onMappedChanged!", Helper.clientName(waylandSurface.surface), mapped)

        // When Socket is enabled and mapped becomes false, set visible
        // after closeAnimation complete， Otherwise set visible directly.
        if (mapped) {
            if (WindowManagementV1.desktopState === WindowManagementV1.Show) {
                WindowManagementV1.desktopState = WindowManagementV1.Normal
                setAllSurfacesVisble(false)
            }

            if (waylandSurface.isMinimized) {
                surface.visible = false;
            } else {
                surface.visible = true;

                if (surface.parent.isCurrentWorkspace)
                    Helper.activatedSurface = waylandSurface

                if (showNewAnimation) {
                    newAnimation.parent = surface.parent
                    newAnimation.anchors.fill = surface
                    newAnimation.sourceComponent = newAnimationComponent
                    newAnimation.item.start()
                }
            }
            workspace().surfaces.appendEnhanced({item: surface, wrapper: surfaceWrapper})
            QmlHelper.workspaceManager.allSurfaces.append({item: surface, wrapper: surfaceWrapper})
        } else { // if not mapped
            if (!waylandSurface.WaylandSocket.rootSocket.enabled) {
                surface.visible = false;
            } else {
                // if don't show CloseAnimation will destroyObject in doDestroy, here is too early
                if (showCloseAnimation) {
                    // do animation for window close
                    closeAnimation.parent = surface.parent
                    closeAnimation.anchors.fill = surface
                    closeAnimation.sourceComponent = closeAnimationComponent
                    closeAnimation.item.start(surface)
                }
            }
            workspace()?.surfaces.removeIf((val) => val.item === surface)
            QmlHelper.workspaceManager.allSurfaces.removeIf((val) => val.item === surface)
            if (Helper.activatedSurface === waylandSurface)
                surface.parent.selectSurfaceToActivate()
        }
    }

    function doDestroy() {
        pendingDestroy = true
        // may have been removed when unmapped?
        workspace().surfaces.removeIf((val) => val.item === surface)
        QmlHelper.workspaceManager.allSurfaces.removeIf((val) => val.item === surface)

        if (!surface.visible || !closeAnimation.active) {
            creator.destroyObject(surfaceWrapper)
            return
        }

        // unbind some properties
        mapped = false
        surface.states = null
        surface.transitions = null

        if (!showCloseAnimation) {
            creator.destroyObject(surfaceWrapper)
        }
    }

    function getPrimaryOutputItem() {
        let output = waylandSurface.surface.primaryOutput
        if (!output)
            return null
        return output.OutputItem.item
    }

    function updateOutputCoordMapper() {
        let output = getPrimaryOutputItem()
        if (!output)
            return

        root.output = output
        root.outputCoordMapper = surface.CoordMapper.helper.get(output)
    }

    Connections {
        id: connOfSurface

        target: waylandSurface
        ignoreUnknownSignals: true

        function onActivateChanged() {
            if (waylandSurface.isActivated) {
                WaylibHelper.itemStackToTop(surface)
                if (surface.effectiveVisible) {
                    Helper.activatedSurface = waylandSurface
                    surface.focus = true;
                }
            }
        }

        function onRequestMove(seat, serial) {
            doMove(seat, serial)
        }

        function onRequestResize(seat, edges, serial) {
            doResize(seat, edges, serial)
        }

        function onRequestMaximize() {
            doMaximize()
        }

        function onRequestCancelMaximize() {
            cancelMaximize()
        }

        function onRequestMinimize() {
            doMinimize()
        }

        function onRequestCancelMinimize() {
            cancelMinimize()
        }

        function onRequestFullscreen() {
            doFullscreen()
        }

        function onRequestCancelFullscreen() {
            cancelFullscreen()
        }
    }

    function doMove(seat, serial) {
        if (waylandSurface.isMaximized)
            return

        Helper.startMove(waylandSurface, surface, seat, serial)
    }

    function doResize(seat, edges, serial) {
        if (waylandSurface.isMaximized)
            return

        Helper.startResize(waylandSurface, surface, seat, edges, serial)
    }

    function doMaximize() {
        if (waylandSurface.isResizeing)
            return

        updateOutputCoordMapper()
        waylandSurface.setMaximize(true)
    }

    function cancelMaximize() {
        if (waylandSurface.isResizeing)
            return

        waylandSurface.setMaximize(false)
    }

    function doMinimize() {
        if (waylandSurface.isResizeing)
            return

        if (waylandSurface.isMinimized)
            return

        surface.focus = false;
        surface.visible = false;
        waylandSurface.setMinimize(true)

        if (Helper.activatedSurface === waylandSurface) {
            Helper.activatedSurface = null
            surface.parent.selectSurfaceToActivate()
        }
    }

    function cancelMinimize () {
        if (waylandSurface.isResizeing)
            return

        if (!waylandSurface.isMinimized)
            return

        if (WindowManagementV1.desktopState === WindowManagementV1.Show) {
            WindowManagementV1.desktopState = WindowManagementV1.Normal
            setAllSurfacesVisble(false)
        }

        Helper.activatedSurface = waylandSurface

        surface.visible = true;
        surface.focus = true;

        waylandSurface.setMinimize(false)
    }

    function doFullscreen() {
        if (waylandSurface.isResizeing)
            return

        updateOutputCoordMapper()
        waylandSurface.setFullScreen(true)
    }

    function cancelFullscreen() {
        if (waylandSurface.isResizeing)
            return

        waylandSurface.setFullScreen(false)
    }

    Component.onCompleted: {
        if (waylandSurface.isMaximized) {
            updateOutputCoordMapper()
        }
    }

    // for workspace management
    Connections {
        target: surface.parent
        function onWorkspaceIdChanged() {
            // sync state to succesive models
            console.log('workspaceIdChanged, reparenting to id=',workspaceId)
        }
    }
    Connections {
        target: surface
        property ToplevelContainer parentCached: { parentCached = target.parent }
        function onParentChanged() {
            parentCached?.surfaces.removeIf((val) => val.item === surface)
            parentCached = target.parent
            target.parent.surfaces.appendEnhanced({item: surface, wrapper: surfaceWrapper})
        }
    }
}
