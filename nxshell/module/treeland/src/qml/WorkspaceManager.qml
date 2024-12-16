import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils


Item {
    id: workspaceManager
    property var wsPanesById: new Map() // The {id : panes[...]} means a 'id' manages a workspace's panes.
    property var wsLayoutById: new Map()
    property list <int> wsList: []

    LoggingCategory {
        id: wmCategory
        name: "com.qt.category"
        defaultLogLevel: LoggingCategory.Info
    }

    function createWs() {
        console.info(wmCategory, "Creating workspace.")
        if (wsList.length === 0) {
            currentWsId = 0
            wsPanesById.set(0, [])
            wsLayoutById.set(0, currentLayout)
            wsList.push(currentWsId)
        } else {
            let size = wsList.length
            wsPanesById.set(wsList[size-1] + 1, [])
            wsLayoutById.set(wsList[size-1] + 1, currentLayout)
            wsList.push(wsList[size-1] + 1)
        }
    }

    function syncWsPanes(opClass, surfaceItem) {
        if (opClass === "addPane") {
            console.info(wmCategory, "Sync the addPane() function.")
            panes.push(wsPanesById.get(currentWsId)[wsPanesById.get(currentWsId).length - 1])
            paneByWs.push(currentWsId)
        } else if (opClass === "removePane") {
            console.info(wmCategory, "Sync the removePane() function.")
            let cnt = 0
            let index = -1
            for (let i = 0; i < paneByWs.length; ++i) {
                if (paneByWs[i] === currentWsId && wsPanesById.get(currentWsId).indexOf(panes[i]) === -1) {
                    // Find the window that was deleted in wsPanes but not deleted in pane
                    index = i
                    break // only delete one pane once.
                }
            }
            panes.splice(index, 1)
            paneByWs.splice(index, 1)
            for (let i = 0; i < paneByWs.length; ++i) {
                if (paneByWs[i] === currentWsId) { // Update the size of the workspace pane in the pane.
                    panes[i].x = wsPanesById.get(currentWsId)[cnt].x
                    panes[i].y = wsPanesById.get(currentWsId)[cnt].y
                    panes[i].height = wsPanesById.get(currentWsId)[cnt].height
                    panes[i].width = wsPanesById.get(currentWsId)[cnt].width
                    ++cnt;
                }
            }
        } else if (opClass === "resizePane" ) {
            console.info(wmCategory, "Sync the resizePane() function.")
            let cnt = 0
            for (let i = 0; i < paneByWs.length; ++i) {
                if (paneByWs[i] === currentWsId) {
                    panes[i].x = wsPanesById.get(currentWsId)[cnt].x
                    panes[i].y = wsPanesById.get(currentWsId)[cnt].y
                    panes[i].height = wsPanesById.get(currentWsId)[cnt].height
                    panes[i].width = wsPanesById.get(currentWsId)[cnt].width
                    ++cnt
                }
            }
        } else if (opClass === "swapPane") {
            console.info(wmCategory, "Sync the swapPane() function.")
            let cnt = 0
            for (let i = 0; i < paneByWs.length; ++i) {
                if (paneByWs[i] === currentWsId) {
                    panes[i].x = wsPanesById.get(currentWsId)[cnt].x
                    panes[i].y = wsPanesById.get(currentWsId)[cnt].y
                    panes[i].height = wsPanesById.get(currentWsId)[cnt].height
                    panes[i].width = wsPanesById.get(currentWsId)[cnt].width
                    ++cnt
                }
            }
            cnt = 0
            for (let i = 0; i < root.panes.length; ++i) {
                if (paneByWs[i] === currentWsId) {
                    root.panes[i].visible = wsPanesById.get(currentWsId)[cnt].visible
                    ++cnt
                }
            }

        } else if (opClass === "reLayout") {
            console.info(wmCategory, "Sync the reLayout() function.")
            let cnt = 0
            for (let i = 0; i < paneByWs.length; ++i) {
                if (panes[i] === surfaceItem) {
                    paneByWs.splice(i, 1)
                    panes.splice(i, 1)
                    break
                }
            }
            for (let i = 0; i < paneByWs.length; ++i) {
                if (paneByWs[i] === currentWsId) {
                    panes[i].x = wsPanesById.get(currentWsId)[cnt].x
                    panes[i].y = wsPanesById.get(currentWsId)[cnt].y
                    panes[i].height = wsPanesById.get(currentWsId)[cnt].height
                    panes[i].width = wsPanesById.get(currentWsId)[cnt].width
                    ++cnt
                }
            }
        }
    }

    function switchNextWs() {
        console.info(wmCategory, "Switching to the next workspace.")
        let index = wsList.indexOf(currentWsId)
        let toId = 0
        if (index === wsList.length - 1) toId = wsList[0];
        else toId = wsList[index+1]
        let fromId = currentWsId
        console.info(wmCategory, "From the workspace", fromId, "to the workspace", toId, ".")
        let fromPanes = wsPanesById.get(fromId)
        for (let i = 0; i < fromPanes.length; ++i) {
            fromPanes[i].visible = false
        }
        let toPanes = wsPanesById.get(toId)
        for (let i = 0; i < toPanes.length; ++i) {
            toPanes[i].visible = true
        }
        currentWsId = toId
        currentLayout = wsLayoutById.get(currentWsId)
        console.info(wmCategory, "currentLayout is", currentLayout)
    }

    function destoryWs() {
        console.info(wmCategory, "Destorying the workspace", currentWsId)
        if (wsPanesById.size === 1) {
            console.info(wmCategory, "You have only one workspace so you cannot destory it.")
            return
        }
        let wsPanes = wsPanesById.get(currentWsId)
        let len = wsPanes.length
        let cnt = 0

        for (let pos = paneByWs.indexOf(currentWsId); pos != -1; pos = paneByWs.indexOf(currentWsId)) {
            Helper.closeSurface(panes[pos].shellSurface.surface)
            deleteFlag.push(1)
            panes.splice(pos, 1)
            paneByWs.splice(pos, 1)
        }
        wsPanes.splice(0, len)
        wsPanesById.delete(currentWsId)
        wsLayoutById.delete(currentWsId)
        let index = wsList.indexOf(currentWsId)
        if (index === wsList.length - 1) {
            wsList.splice(index, 1)
            currentWsId = wsList[0]
        } else {
            currentWsId = wsList[index+1]
            wsList.splice(index, 1)
        }
        for (let i = 0; i < wsPanesById.get(currentWsId).length; ++i) {
            wsPanesById.get(currentWsId)[i].visible = true
        }
    }

    // TODO: fulfil the moveWs() function.
    function moveWs(currentWsId, toWsId) {
        let surfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
    }
}
