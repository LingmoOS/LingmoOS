import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    id: horizontalLayout

    property int spacing: 0 // Distance between this pane and that pane, temporarily set to 0.
    property Item selectSurfaceItem1: null
    property Item selectSurfaceItem2: null
    property Item currentSurfaceItem: null

    LoggingCategory {
        id: hCategory
        name: "treeland.tiledlayout.horizontal"
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
            wsPanes[0].x = 0
            wsPanes[0].width *= scale
            wsPanes[0].visible = true
            for (let i = 1; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                wsPanes[i].y = wsPanes[0].y
                wsPanes[i].width *= scale
                wsPanes[i].height = wsPanes[0].height
                wsPanes[i].visible = true
            }
            surfaceItem.x = wsPanes[wsPanes.length - 1].x + wsPanes[wsPanes.length - 1].width
            surfaceItem.y = wsPanes[0].y
            surfaceItem.width = root.width * (1.0 - scale)
            surfaceItem.height = wsPanes[0].height
            surfaceItem.visible = true
            wsPanes.push(surfaceItem)
        }
        workspaceManager.syncWsPanes("addPane")
    }

    // TODO: Update the ui of choosing logic.
    function choosePane(id) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        currentSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        if (id === 1) {
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem1 = wsPanes[0]
            } else {
                selectSurfaceItem1 = wsPanes[index+1]
            }
            console.info(hCategory, "The first selected pane is index", wsPanes.indexOf(selectSurfaceItem1))
            Helper.activatedSurface = selectSurfaceItem1.shellSurface
        } else if (id === 2) {
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem2 = wsPanes[0]
            } else {
                selectSurfaceItem2 = wsPanes[index+1]
            }
            console.info(hCategory, "The second selected pane is index", wsPanes.indexOf(selectSurfaceItem2))
            Helper.activatedSurface = selectSurfaceItem2.shellSurface
        }
    }

    function removePane(removeSurfaceIf) {
        let surfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(surfaceItem)
        if (wsPanes.length === 1) {
            wsPanes.splice(index, 1)
            // It is no necessory to modify the height or y when removing a pane.
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if(removeSurfaceIf === 0) surfaceItem.visible = false
        } else {
            let scale = wsPanes.length / (wsPanes.length - 1)
            wsPanes.splice(index, 1)
            wsPanes[0].x = 0
            wsPanes[0].width *= scale  // Magnification factor.
            for (let i = 1; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                wsPanes[i].y = wsPanes[0].y
                wsPanes[i].width *= scale
                wsPanes[i].height = wsPanes[0].height
            }
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if(removeSurfaceIf === 0) surfaceItem.visible = false
        }
        workspaceManager.syncWsPanes("removePane")
    }

    // Direction: 1 = Left, 2 = Right, 3 = Up, 4 = Down.
    function resizePane(size, direction) {
        let activeSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(activeSurfaceItem)

        if (wsPanes.length === 1) {
            console.info(hCategory, "Only one pane, cannot call resizePane() function.")
            return
        }
        if (index === 0) {
            let delta = size / (wsPanes.length - 1)
            if (direction === 1) {
                console.info(hCategory, "You cannot left anymore!")
                return
            } else if (direction === 2) {
                activeSurfaceItem.width += size
                for (let i = 1; i < wsPanes.length; ++i) {
                    wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                    wsPanes[i].width -= delta
                }
            }
        } else {
            let delta = size / index
            let last = wsPanes.length - 1

            if (index === last) {
                if (direction === 1) {
                    wsPanes[last].width -= size
                    wsPanes[last].x += size
                    for (let i = last - 1; i >= 0; --i) {
                        wsPanes[i].width += delta
                        wsPanes[i].x = wsPanes[i+1].x - wsPanes[i].width
                    }
                    wsPanes[0].x = 0
                } else if (direction === 2) {
                    console.info(hCategory, "You cannot left anymore!")
                    return
                }
            } else if (direction === 1) {
                wsPanes[0].x = 0
                wsPanes[0].width += delta
                for (let i = 1; i < index; ++i) {
                    wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                    wsPanes[i].width += delta
                }
                activeSurfaceItem.x += size
                activeSurfaceItem.width -= size
            } else if (direction === 2) {
                wsPanes[last].x += delta
                wsPanes[last].width -= delta
                for (let i = last - 1; i > index; i--) {
                    wsPanes[i].width -= delta
                    wsPanes[i].x = wsPanes[i+1].x - wsPanes[i].width
                }
                activeSurfaceItem.x = activeSurfaceItem.x // x donnot change.
                activeSurfaceItem.width += size
            }
        }
        workspaceManager.syncWsPanes("resizePane")
    }

    function swapPane() {
        if (selectSurfaceItem1 === null || selectSurfaceItem2 === null) {
            console.info(hCategory, "You should choose two pane before swap them.")
            return
        }

        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let activeSurfaceItem = selectSurfaceItem1
        let targetSurfaceItem = selectSurfaceItem2
        let index1 = wsPanes.indexOf(activeSurfaceItem)
        let index2 = wsPanes.indexOf(targetSurfaceItem)

        if (index1 === index2) {
            console.info(hCategory, "You should choose two different panes to swap.")
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

        let delta = activeSurfaceItem.width - targetSurfaceItem.width

        if (index1 < index2) {
            if (index2 - index1 === 1) {
                targetSurfaceItem.x = activeSurfaceItem.x
                activeSurfaceItem.x = targetSurfaceItem.x + targetSurfaceItem.width
            } else {
                let tempPane = targetSurfaceItem
                tempPane.x = activeSurfaceItem.x
                wsPanes[index1+1].x = tempPane.x + tempPane.width
                for (let i = index1 + 2; i < index2; ++i) {
                    wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                }
                targetSurfaceItem = tempPane
                activeSurfaceItem.x = wsPanes[index2-1].x + wsPanes[index2-1].width
            }
        } else if (index1 > index2) {
            if (index1 - index2 === 1) {
                activeSurfaceItem.x = targetSurfaceItem.x
                targetSurfaceItem.x = activeSurfaceItem.x + activeSurfaceItem.width
            } else {
                let tempPane = activeSurfaceItem
                tempPane.x = targetSurfaceItem.x
               wsPanes[index2+1].x = tempPane.x + tempPane.width
                for (let i = index2 + 2; i < index1; ++i) {
                    wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                }
                activeSurfaceItem = tempPane
                targetSurfaceItem.x = wsPanes[index1-1].x + wsPanes[index1-1].width
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
        let index = wsPanes.indexOf(surfaceItem)
        if (wsPanes.length === 1) {
            wsPanes.splice(index, 1)
        } else {
            let scale = wsPanes.length / (wsPanes.length - 1)
            wsPanes.splice(index, 1)
            wsPanes[0].x = 0
            wsPanes[0].width *= scale
            for (let i = 1; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[i-1].x + wsPanes[i-1].width
                wsPanes[i].y = wsPanes[0].y
                wsPanes[i].width *= scale
                wsPanes[i].height = wsPanes[0].height
            }
        }
        workspaceManager.syncWsPanes("reLayout", surfaceItem)
    }

}
