import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils


Item {
    id: verticalLayout

    property int spacing: 0 // Distance between this pane and that pane, temporarily set to 0.
    property Item selectSurfaceItem1: null
    property Item selectSurfaceItem2: null
    property Item currentSurfaceItem: null

    LoggingCategory {
        id: vCategory
        name: "treeland.tiledlayout.vertical"
        defaultLogLevel: LoggingCategory.Info
    }

    function addPane(surfaceItem) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        if (wsPanes.length === 0) {
            surfaceItem.x = 0
            surfaceItem.y = 0
            surfaceItem.width = root.width
            surfaceItem.height = root.height
            surfaceItem.visible = true
            wsPanes.push(surfaceItem)
        } else {
            let scale = wsPanes.length / (wsPanes.length + 1)
            wsPanes[0].y = 0
            wsPanes[0].height *= scale
            wsPanes[0].visible = true
            for (let i = 1; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[0].x
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                wsPanes[i].height *= scale
                wsPanes[i].width = wsPanes[0].width
                wsPanes[i].visible = true
            }
            surfaceItem.x = wsPanes[0].x
            surfaceItem.y = wsPanes[wsPanes.length - 1].y + wsPanes[wsPanes.length - 1].height
            surfaceItem.height = root.height * (1.0 - scale)
            surfaceItem.width = wsPanes[0].width
            surfaceItem.visible = true
            wsPanes.push(surfaceItem)
        }
        workspaceManager.syncWsPanes("addPane")
    }


    // TODO: Update the ui of choosing logic.
    function choosePane(id) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        currentSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        if (id === 1) { // selected for selectPane1.
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem1 = wsPanes[0]
            } else {
                selectSurfaceItem1 = wsPanes[index+1]
            }
            console.info(vCategory, "The first selected pane is index", wsPanes.indexOf(selectSurfaceItem1))
            Helper.activatedSurface = selectSurfaceItem1.shellSurface
        } else if (id === 2) { // selectd for selectPane2.
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem2 = wsPanes[0]
            } else {
                selectSurfaceItem2 = wsPanes[index+1]
            }
            console.info(vCategory, "The second selected pane is index", wsPanes.indexOf(selectSurfaceItem2))
            Helper.activatedSurface = selectSurfaceItem2.shellSurface
        }
    }

    function removePane(removeSurfaceIf) {
        console.info(vCategory, "vertical layout remove pane.")
        let surfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(surfaceItem)
        if (wsPanes.length === 1) {
            wsPanes.splice(index, 1)
            // It is no necessory to modify the height or y when removing a pane.
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        } else {
            let scale = wsPanes.length / (wsPanes.length - 1) // Magnification factor.
            wsPanes.splice(index, 1)
            wsPanes[0].y = 0
            wsPanes[0].height *= scale
            for (let i = 1; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[0].x
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                wsPanes[i].height *= scale
                wsPanes[i].width = wsPanes[0].width
            }
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        }
        workspaceManager.syncWsPanes("removePane")
    }

    // Direction: 1 = Left, 2 = Right, 3 = Up, 4 = Down.
    function resizePane(size, direction) {
        console.info(vCategory, "vertical layout resize pane.")
        let activeSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(activeSurfaceItem)

        if (wsPanes.length === 1) {
            console.info(vCategory, "Only one pane, cannot call resizePane().")
            return
        }


        if (index === 0) {
            let delta = size / (wsPanes.length - 1)
            if (direction === 3) {
                console.info(vCategory, "You cannot up anymore!")
                return
            } else if (direction === 4) {
                activeSurfaceItem.height += size
                for (let i = 1; i < wsPanes.length; i++) {
                    wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height;
                    wsPanes[i].height -= delta
                }
            }
        } else {
            let delta = size / index
            let last = wsPanes.length - 1
            if (index === last) {
                if (direction === 3) {
                    wsPanes[last].height -= size
                    wsPanes[last].y += size
                    for (let i = last - 1; i >= 0; --i) {
                        wsPanes[i].height += delta
                        wsPanes[i].y = wsPanes[i+1].y - wsPanes[i].height
                    }
                    wsPanes[0].y = 0
                } else if (direction === 4) {
                    console.info(vCategory, "You cannot down anymore!")
                    return
                }
            } else if (direction === 3) {
                wsPanes[0].y = 0
                wsPanes[0].height += delta
                // the middle of panes
                for (let i = 1; i < index; ++i) {
                    wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                    wsPanes[i].height += delta
                }
                // current pane
                activeSurfaceItem.y += size
                activeSurfaceItem.height -= size
            } else if (direction === 4) {
                // the last of pane
                wsPanes[last].y += delta
                wsPanes[last].height -= delta
                for (let i = last - 1; i > index; i--) {
                    wsPanes[i].height -= delta
                    wsPanes[i].y = wsPanes[i+1].y - wsPanes[i].height
                }
                activeSurfaceItem.y = activeSurfaceItem.y // y is not change
                activeSurfaceItem.height += size
            }
        }
        workspaceManager.syncWsPanes("resizePane")
    }

    function swapPane() {
        console.info(vCategory, "vertical layout swap pane")
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        if (selectSurfaceItem1 === null || selectSurfaceItem2 === null) {
            console.info(vCategory, "You should choose two panes before swap them.")
            return
        }

        let activeSurfaceItem = selectSurfaceItem1
        let targetSurfaceItem = selectSurfaceItem2
        let index1 = wsPanes.indexOf(activeSurfaceItem)
        let index2 = wsPanes.indexOf(targetSurfaceItem)
        if (index1 === index2) {
            console.info(vCategory, "You should choose two different panes to swap.")
            return
        }
        let tempIndex1 = null
        let tempIndex2 = null
        for (let i = 0; i < root.panes.length; ++i) {
            if (wsPanes[index1] === root.panes[i] && root.paneByWs[i] === currentWsId) {
                tempIndex1 = i
            }
            if (wsPanes[index2] === root.panes[i] && root.paneByWs[i] === currentWsId) {
                tempIndex2 = i
            }
        }

        if (index1 < index2) {
            if (index2 - index1 === 1) {
                targetSurfaceItem.y = activeSurfaceItem.y
                activeSurfaceItem.y = targetSurfaceItem.y + targetSurfaceItem.height
            } else {
                let tempPane = targetSurfaceItem
                tempPane.y = activeSurfaceItem.y
                wsPanes[index1+1].y = tempPane.y + tempPane.height
                for (let i = index1 + 2; i < index2; ++i) {
                    wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                }
                targetSurfaceItem = tempPane
                activeSurfaceItem.y = wsPanes[index2-1].y + wsPanes[index2-1].height
            }
        } else if (index1 > index2) {
            if (index1 - index2 === 1) {
                activeSurfaceItem.y = targetSurfaceItem.y
                targetSurfaceItem.y = activeSurfaceItem.y + activeSurfaceItem.height
            } else {
                let tempPane = activeSurfaceItem
                tempPane.y = targetSurfaceItem.y
               wsPanes[index2+1].y = tempPane.y + tempPane.height
                for (let i = index2 + 2; i < index1; ++i) {
                    wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                }
                activeSurfaceItem = tempPane
                targetSurfaceItem.y = wsPanes[index1-1].y + wsPanes[index1-1].height
            }
        }

        let temp1 = root.panes[tempIndex1]
        root.panes[tempIndex1] = root.panes[tempIndex2]
        root.panes[tempIndex2] = temp1
        let temp2 = wsPanes[index1];
        wsPanes[index1] = wsPanes[index2];
        wsPanes[index2] = temp2;

        workspaceManager.syncWsPanes("swapPane")
    }

    function reLayout(surfaceItem) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let scale = wsPanes.length / (wsPanes.length - 1)
        let index = wsPanes.indexOf(surfaceItem)
        wsPanes.splice(index, 1)
        wsPanes[0].y = 0
        wsPanes[0].height *= scale
        for (let i = 1; i < wsPanes.length; ++i) {
            wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
            wsPanes[i].height *= scale
        }
        workspaceManager.syncWsPanes("reLayout", surfaceItem)
    }
}
