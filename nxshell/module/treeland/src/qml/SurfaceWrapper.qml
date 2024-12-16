// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Utils

SurfaceItemFactory {
    id: root

    required property ToplevelSurface waylandSurface
    property string type

    property var doDestroy: helper.doDestroy
    property var cancelMinimize: helper.cancelMinimize
    property var personalizationMapper: PersonalizationV1.Attach(waylandSurface)
    property int outputCounter: 0
    property alias toplevelSurfaceItem: root.surfaceItem
    property alias decoration: decoration
    readonly property XWaylandSurfaceItem asXwayland: {surfaceItem as XWaylandSurfaceItem}
    readonly property XdgSurfaceItem asXdg: {surfaceItem as XdgSurfaceItem}
    z: {
        if (Helper.clientName(waylandSurface.surface) === "ocean-desktop") {
            return -100 + 1
        }
        else if (Helper.clientName(waylandSurface.surface) === "ocean-launchpad") {
            return 25
        }
        else {
            return 0
        }
    }

    function move(pos) {
        manualMoveResizing = true
        surfaceItem.x = pos.x
        surfaceItem.y = pos.y
        manualMoveResizing = false
    }

    required property int workspaceId
    // put here, otherwise may initialize late
    parent: QmlHelper.workspaceManager.workspacesById.get(workspaceId)

    surfaceItem {
        parent: root.parent
        shellSurface: waylandSurface
        topPadding: decoration.visible ? decoration.topMargin : 0
        bottomPadding: decoration.visible ? decoration.bottomMargin : 0
        leftPadding: decoration.visible ? decoration.leftMargin : 0
        rightPadding: decoration.visible ? decoration.rightMargin : 0
        focus: toplevelSurfaceItem.shellSurface === Helper.activatedSurface
        resizeMode:
            if (!surfaceItem.effectiveVisible)
                SurfaceItem.ManualResize
            else if (stateTransition.running
                    || waylandSurface.isMaximized)
                SurfaceItem.SizeToSurface
            else
                SurfaceItem.SizeFromSurface
        onResizeModeChanged: {
            // if surface mapped when not visible, it will change mode to sizefromsurf
            // but mode change is not applied if no resize event happens afterwards, so trigger resize here
            if (surfaceItem.resizeMode != SurfaceItem.ManualResize)
                surfaceItem.resize(surfaceItem.resizeMode)
        }
        onEffectiveVisibleChanged: {
            if (surfaceItem.effectiveVisible) {
                console.assert(surfaceItem.resizeMode !== SurfaceItem.ManualResize,
                               "The surface's resizeMode Shouldn't is ManualResize")
                // Apply the WSurfaceItem's size to wl_surface
                surfaceItem.resize(SurfaceItem.SizeToSurface)
            } else {
                Helper.cancelMoveResize(surfaceItem)
            }
        }
        transitions: Transition {
            id: stateTransition
            NumberAnimation {
                properties: "x,y,width,height"
                duration: 100
            }
        }
        states: [
            State {
                name: "" // avoid applying propertychange on initialize
                PropertyChanges {
                    id: defulatState
                    restoreEntryValues: false
                    target: toplevelSurfaceItem
                    x: store.normal.x
                    y: store.normal.y
                    width: store.normal.width
                    height: store.normal.height
                }
            },
            State {
                name: "intermediate"
                PropertyChanges {
                    restoreEntryValues: false
                    target: toplevelSurfaceItem
                    x: store.normal.x
                    y: store.normal.y
                    width: store.normal.width
                    height: store.normal.height
                }
            },
            State {
                name: "maximize"
                when: helper.isMaximize
                PropertyChanges {
                    restoreEntryValues: true
                    target: toplevelSurfaceItem
                    x: helper.maximizeRect.x
                    y: helper.maximizeRect.y
                    width: helper.maximizeRect.width
                    height: helper.maximizeRect.height
                }
            },
            State {
                name: "fullscreen"
                when: helper.isFullScreen
                PropertyChanges {
                    restoreEntryValues: true
                    target: toplevelSurfaceItem
                    x: helper.fullscreenRect.x
                    y: helper.fullscreenRect.y
                    z: Helper.clientName(waylandSurface.surface) == "ocean-launchpad" ? 25 : 100 + 1 // LayerType.Overlay + 1
                    width: helper.fullscreenRect.width
                    height: helper.fullscreenRect.height
                }
            }
        ]
    }

    property var store: {}
    property int storeNormalWidth: undefined
    property bool isRestoring: false
    property bool aboutToRestore: false
    function saveState() {
        let nw = store ?? {}
        console.debug(`store = ${store} state = {x: ${surfaceItem.x}, y: ${surfaceItem.y}, width: ${surfaceItem.width}, height: ${surfaceItem.height}}`)
        nw.normal = {x: surfaceItem.x, y: surfaceItem.y, width: surfaceItem.width, height: surfaceItem.height}
        store = nw
    }
    function restoreState(toStore) {
        console.debug(`restoring state to ${QmlHelper.printStructureObject(toStore.normal)}`)
        surfaceItem.state = "intermediate"
        store = toStore
        surfaceItem.state = ""
    }
    onIsMoveResizingChanged: if (!isMoveResizing) {
        if (state == "")
            saveState()
    }
    onStoreChanged: {
        storeNormalWidth = store.normal.width
    }
    Component.onCompleted: {
        saveState() // save initial state
    }

    OutputLayoutItem {
        parent: surfaceItem
        anchors.fill: parent
        layout: Helper.outputLayout

        onEnterOutput: function(output) {
            if (waylandSurface.surface) {
                waylandSurface.surface.enterOutput(output)
            }
            Helper.onSurfaceEnterOutput(waylandSurface, toplevelSurfaceItem, output)
            outputCounter++

            if (outputCounter == 1) {
                const pos = QmlHelper.winposManager.nextPos(waylandSurface.appId, toplevelSurfaceItem.parent, toplevelSurfaceItem)
                let outputDelegate = output.OutputItem.item
                move(pos)

                if (Helper.clientName(waylandSurface.surface) === "ocean-desktop") {
                    toplevelSurfaceItem.x = outputDelegate.x
                    toplevelSurfaceItem.y = outputDelegate.y
                    toplevelSurfaceItem.width = outputDelegate.width
                    toplevelSurfaceItem.height = outputDelegate.height
                }

                if (Helper.clientName(waylandSurface.surface) === "ocean-launchpad") {
                    toplevelSurfaceItem.x = outputDelegate.x
                    toplevelSurfaceItem.y = outputDelegate.y
                }
            }
        }
        onLeaveOutput: function(output) {
            waylandSurface.surface.leaveOutput(output)
            Helper.onSurfaceLeaveOutput(waylandSurface, toplevelSurfaceItem, output)
            outputCounter--

            if (outputCounter == 0 && helper.mapped) {
                console.log(`nextPos when output=0 ${waylandSurface} ${toplevelSurfaceItem}`)
                const pos = QmlHelper.winposManager.nextPos(waylandSurface.appId, toplevelSurfaceItem.parent, toplevelSurfaceItem)
                toplevelSurfaceItem.move(pos)
            }
        }
    }

    WindowDecoration {
        id: decoration
        parent: surfaceItem
        anchors.fill: parent
        z: SurfaceItem.ZOrder.ContentItem - 1
        surface: toplevelSurfaceItem.shellSurface
        visible: enable && !helper.isFullScreen
        moveable: !helper.isMaximize && !helper.isFullScreen
        radius: helper.isMaximize ? 0 : 15
    }

    StackToplevelHelper {
        id: helper
        surface: surfaceItem
        waylandSurface: toplevelSurfaceItem.shellSurface
        creator: toplevelComponent
        decoration: decoration
        surfaceWrapper: root
    }

    property bool manualMoveResizing: false
    property bool isMoveResizing: manualMoveResizing || Helper.resizingItem == surfaceItem || Helper.movingItem == surfaceItem
}
