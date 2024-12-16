// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    id: root
    function getSurfaceItemFromWaylandSurface(surface) {
        let finder = function(props) {
            if (!props.waylandSurface)
                return false
            // surface is WToplevelSurface or WSurfce
            if (props.waylandSurface === surface || props.waylandSurface.surface === surface)
                return true
        }

        let toplevel = Helper.xdgShellCreator.getIf(toplevelComponent, finder)
        if (toplevel) {
            return {
                shell: toplevel,
                item: toplevel,
                type: "toplevel"
            }
        }

        let popup = Helper.xdgShellCreator.getIf(popupComponent, finder)
        if (popup) {
            return {
                shell: popup,
                item: popup.xdgSurface,
                type: "popup"
            }
        }

        let layer = Helper.layerShellCreator.getIf(layerComponent, finder)
        if (layer) {
            return {
                shell: layer,
                item: layer.surfaceItem,
                type: "layer"
            }
        }

        // let xwayland = Helper.xwaylandCreator.getIf(xwaylandComponent, finder)
        // if (xwayland) {
        //     return {
        //         shell: xwayland,
        //         item: xwayland,
        //         type: "xwayland"
        //     }
        // }

        return null
    }

    LoggingCategory {
        id: category
        name: "treeland.tiledlayout"
        defaultLogLevel: LoggingCategory.Info
    }

    WorkspaceManager {
        id: workspaceManager
        anchors.fill: parent
    }


    SlideLayout {
        id: slideLayout
        // objectName: "slideLayout"
        anchors.fill: parent
    }

    HorizontalLayout {
        id: horizontalLayout
        anchors.fill: parent
    }

    VerticalLayout {
        id: verticalLayout
        anchors.fill: parent
    }

    TallLayout {
        id : tallLayout
        anchors.fill: parent
    }

    function getPanes(wsId) {
        return workspaceManager.wsPanesById.get(wsId)
    }

    property list <XdgSurfaceItem> panes: [] // Manage all panels.
    property list <int> paneByWs: [] // Which WS does the i-th pane belong to.
    property Item currentLayout: verticalLayout // Initialize default layout.
    property int currentWsId: -1 // currentWorkSpace id.
    property list <int> deleteFlag: [] // This flag is used to indicate whether this pane has been removed by removePane() function.
    property list <Item> layouts: [slideLayout, verticalLayout, horizontalLayout, tallLayout]


    Connections {
        target: Helper
        function onResizePane(size, direction) {
            console.info(category, "the move size is", size, ", and the move direction is", direction)
            if (currentLayout === slideLayout) {
                console.log(category, "This Layout don't have resizePane() function!")
                return
            }
            if (currentLayout === verticalLayout && (direction === 1 || direction === 2)) {
                console.log(category, "This Layout cannot left or right anymore!")
                return
            }
            if (currentLayout === horizontalLayout && (direction === 3 || direction === 4)) {
                console.log(category, "This Layout cannot up or down anymore!")
                return
            }
            currentLayout.resizePane(size, direction)
        }
        function onSwapPane() { currentLayout.swapPane() }
        function onRemovePane(removeSurfaceIf) { currentLayout.removePane(removeSurfaceIf) }
        function onChoosePane(id) { currentLayout.choosePane(id) }
        function onSwitchLayout() { switchLayout() }

        function onCreateWs() { workspaceManager.createWs() }
        function onDestoryWs() { workspaceManager.destoryWs() }
        function onSwitchNextWs() { workspaceManager.switchNextWs() }
        function onMoveWs(wsId) { --wsId; workspaceManager.moveWs(currentWsId, wsId) }
    }

    function switchLayout() {
        console.info(category, "switchLayout")
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = layouts.indexOf(currentLayout)
        let len = wsPanes.length
        index += 1
        if (index === layouts.length) {
            index = 0
        }
        let oldLayout = currentLayout
        let tempPanes = []
        for (let i = 0; i < len; ++i) {
            tempPanes.push(wsPanes[i])
        }
        currentLayout = layouts[index]
        for (let i = 0; i < len; ++i) {
            Helper.activatedSurface = wsPanes[0].shellSurface
            oldLayout.removePane(0)
        }
        for (let i = 0; i < tempPanes.length; ++i) {
            currentLayout.addPane(tempPanes[i])
        }
        workspaceManager.wsLayoutById.set(currentWsId, currentLayout)

    }

    DynamicCreatorComponent {
        id: toplevelComponent
        creator: Helper.xdgShellCreator
        chooserRole: "type"
        chooserRoleValue: "toplevel"
        autoDestroy: false

        onObjectRemoved: function (obj) {
            obj.doDestroy()
        }

        XdgSurfaceItem {
            id: toplevelVerticalSurfaceItem
            resizeMode: SurfaceItem.SizeToSurface
            property var doDestroy: helper.doDestroy

            required property WaylandXdgSurface waylandSurface
            property string type
            shellSurface: waylandSurface

            Component.onCompleted: {
                if (currentWsId === -1) {
                    workspaceManager.createWs(currentLayout)
                }
                // paneByWs.push(currentWsId)
                currentLayout.addPane(toplevelVerticalSurfaceItem)
            }

            Component.onDestruction: {
                if (deleteFlag.indexOf(1) != -1) {
                    deleteFlag.splice(0, 1)
                    return
                }
                currentLayout.reLayout(toplevelVerticalSurfaceItem)
            }

            OutputLayoutItem {
                anchors.fill: parent
                layout: Helper.outputLayout

                onEnterOutput: function(output) {
                    waylandSurface.surface.enterOutput(output)
                    Helper.onSurfaceEnterOutput(waylandSurface, toplevelVerticalSurfaceItem, output)
                }
                onLeaveOutput: function(output) {
                    waylandSurface.surface.leaveOutput(output)
                    Helper.onSurfaceLeaveOutput(waylandSurface, toplevelVerticalSurfaceItem, output)
                }
            }

            TiledToplevelHelper {
                id: helper
                surface: toplevelVerticalSurfaceItem
                waylandSurface: toplevelVerticalSurfaceItem.waylandSurface
                creator: toplevelComponent
            }
        }
    }

    DynamicCreatorComponent {
        id: layerComponent
        creator: Helper.layerShellCreator
        autoDestroy: false

        onObjectRemoved: function (obj) {
            obj.doDestroy()
        }

        LayerSurface {
            id: layerSurface
            creator: layerComponent
        }
    }

    DynamicCreatorComponent {
        id: inputPopupComponent
        creator: Helper.inputPopupCreator

        InputPopupSurface {
            required property WaylandInputPopupSurface popupSurface

            parent: getSurfaceItemFromWaylandSurface(popupSurface.parentSurface)
            id: inputPopupSurface
            shellSurface: popupSurface
        }
    }
}
