import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    id: slideLayout

    property int spacing: 0 // Distance between this pane and that pane, temporarily set to 0.

    LoggingCategory {
        id: sCategory
        name: "com.qt.category"
        defaultLogLevel: LoggingCategory.Info
    }

    function addPane(surfaceItem) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        surfaceItem.x = 0
        surfaceItem.y = 0
        surfaceItem.width = root.width
        surfaceItem.height = root.height
        surfaceItem.visible = true
        wsPanes.push(surfaceItem)
        workspaceManager.syncWsPanes("addPane")
    }

    function swapPane() {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let currentSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        if (wsPanes.length === 1) {
            console.info(sCategory, "You have only one pane.")
            return
        }
        let index = wsPanes.indexOf(currentSurfaceItem)
        index += 1
        if (index === wsPanes.length) {
            index = 0
        }
        for (let i = 0; i < wsPanes.length; ++i) {
            if (i === index) wsPanes[i].visible = true
            else wsPanes[i].visible = false
        }
        Helper.activatedSurface = wsPanes[index].shellSurface
        workspaceManager.syncWsPanes("swapPane")
    }

    function removePane(removeSurfaceIf) {
        // bug: If there is only one window left under slideLayout,
        // deleting it will result in error 116: ReferenceError: xwaylandComponent is not defined.
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let surfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let index = wsPanes.indexOf(surfaceItem)
        wsPanes.splice(index, 1)
        if (removeSurfaceIf) {
            Helper.closeSurface(surfaceItem.shellSurface.surface)
            deleteFlag.push(1)
        } else if(!removeSurfaceIf) surfaceItem.visible = false
        if (wsPanes.length) {
            let vis = -1
            for (let i = 0; i < wsPanes.length; ++i) {
                if (wsPanes[i].visible === true) { vis = 1; break}
            }
            if (vis === -1) wsPanes[0].visible = true
        }
        workspaceManager.syncWsPanes("removePane")
    }

    function reLayout(surfaceItem) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(surfaceItem)
        wsPanes.splice(index, 1)
        workspaceManager.syncWsPanes("reLayout", surfaceItem)
    }
}
