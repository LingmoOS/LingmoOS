import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

Item {
    id: tallLayout

    property int spacing: 0 // Distance between this pane and that pane, temporarily set to 0.
    property Item selectSurfaceItem1: null
    property Item selectSurfaceItem2: null
    property Item currentSurfaceItem: null

    LoggingCategory {
        id: tCategory
        name: "treeland.tiledlayout.tall"
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
        } else if (wsPanes.length === 1) {
            wsPanes[0].width = root.width / 2
            wsPanes[0].height = root.height
            wsPanes[0].visible = true
            surfaceItem.x = wsPanes[0].x + wsPanes[0].width
            surfaceItem.y = wsPanes[0].y
            surfaceItem.width = root.width / 2
            surfaceItem.height = wsPanes[0].height
            surfaceItem.visible = true
            wsPanes.push(surfaceItem)
        } else {
            // There are two or more panes, split vertically on the right side,
            // and processed according to the VerticalLayout.
            let scale = (wsPanes.length - 1) / (wsPanes.length)
            wsPanes[0].visible = true
            wsPanes[1].y = 0
            wsPanes[1].height *= scale
            wsPanes[1].visible = true
            for (let i = 2; i < wsPanes.length; i++) {
                wsPanes[i].x = wsPanes[i-1].x
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                wsPanes[i].width = wsPanes[i-1].width
                wsPanes[i].height *= scale
                wsPanes[i].visible = true
            }
            surfaceItem.x = wsPanes[wsPanes.length - 1].x
            surfaceItem.y = wsPanes[wsPanes.length - 1].y + wsPanes[wsPanes.length - 1].height
            surfaceItem.height = root.height - surfaceItem.y
            surfaceItem.width = wsPanes[wsPanes.length - 1].width
            surfaceItem.visible = true
            wsPanes.push(surfaceItem)
        }
        workspaceManager.syncWsPanes("addPane")
    }


    function choosePane(id) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        currentSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        if (id === 1) { // selected for selectPane1
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem1 = wsPanes[0]
            } else {
                selectSurfaceItem1 = wsPanes[index+1]
            }
            console.info(tCategory, "The first selected pane is index", wsPanes.indexOf(selectSurfaceItem1))
            Helper.activatedSurface = selectSurfaceItem1.shellSurface
        } else if (id === 2) { // selectd for selectPane2
            let index = wsPanes.indexOf(currentSurfaceItem)
            if (index === wsPanes.length - 1) {
                selectSurfaceItem2 = wsPanes[0]
            } else {
                selectSurfaceItem2 = wsPanes[index+1]
            }
            console.info(tCategory, "The second selected pane is index", wsPanes.indexOf(selectSurfaceItem2))
            Helper.activatedSurface = selectSurfaceItem2.shellSurface
        }
    }

    // Direction: 1 = Left, 2 = Right, 3 = Up, 4 = Down.
    function resizePane(size, direction) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let activeSurfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let index = wsPanes.indexOf(activeSurfaceItem)
        if (wsPanes.length === 1) {
            console.info(tCategory, "Only one pane, cannot call resizePane() function.")
            return
        }
        if (index === 0) { // The first pane is on the left.
            if (direction === 1) {
                console.info(tCategory, "You cannot resize the first pane on left!")
                return
            } else if (direction === 2) {
                activeSurfaceItem.width += size
                for (let i = 1; i < wsPanes.length; i++) {
                    wsPanes[i].x += size
                    wsPanes[i].width -= size
                }
            }
        } else { // The rest panes are in the right pane.
            if (direction === 3 || direction === 4) {
                let last = wsPanes.length - 1
                if (index === 1) { // The first pane on the right.
                    if (direction === 3) { // Move the upper line to adjust the size.
                        console.info(tCategory, "You cannot resize the first pane by the upper line!")
                        return
                    } else if (direction === 4) { // Adjust the size with the lower edge line.
                        let delta = size / (wsPanes.length - index - 1)
                        wsPanes[1].height += size
                        for (let i = 2; i < wsPanes.length; i++) {
                            wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                            wsPanes[i].height -= delta
                        }
                    }
                } else if (index === last) { // The last pane on the right.
                    if (direction === 4) {
                        console.info(tCategory, "You cannot resize the last pane by the lower line!")
                        return
                    } else if (direction === 3) { // Adjust the size with the upper line.
                        let delta = size / (index - 1)
                        wsPanes[last].height -= size
                        wsPanes[last].y += size
                        for (let i = last - 1; i >= 1; i--) {
                            wsPanes[i].height += delta
                            wsPanes[i].y = wsPanes[i+1].y - wsPanes[i].height
                        }
                        wsPanes[1].y = 0
                    }
                } else if (index > 1 && index < last) { // In the middle pane on the right.
                    if (direction === 3) { // Adjust the size with the upper line.
                        let delta = size / (index - 1)
                        wsPanes[1].y = 0
                        wsPanes[1].height += delta
                        for (let i = 2; i < index; i++) {
                            wsPanes[i].height += delta
                            wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                        }
                        activeSurfaceItem.y += size
                        activeSurfaceItem.height -= size
                    } else if (direction === 4) { // Adjust the size with the lower edge line.
                        let delta = size / (wsPanes.length - index - 1)
                        wsPanes[last].height -= delta
                        wsPanes[last].y += delta
                        for (let i = last - 1; i > index; i--) {
                            wsPanes[i].height -= delta
                            wsPanes[i].y = wsPanes[i+1].y - wsPanes[i].height
                        }
                        activeSurfaceItem.height += size
                    }
                }
            } else if (direction === 1) {
                wsPanes[0].width += size
                for (let i = 1; i < wsPanes.length; i++) {
                    wsPanes[i].x += size
                    wsPanes[i].width -= size
                }
            } else if (direction === 2) {
                console.info(tCategory, "You cannot resize the pane by the right line!")
                return
            }
        }
        workspaceManager.syncWsPanes("resizePane")
    }

    function removePane(removeSurfaceIf) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        if (root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item === null) {
            console.info(tCategory, "You should choose a pane firstly.")
            return
        }

        let surfaceItem = root.getSurfaceItemFromWaylandSurface(Helper.activatedSurface).item
        let index = wsPanes.indexOf(surfaceItem)

        if (wsPanes.length === 1) {
            wsPanes.splice(index, 1)
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        } else if (wsPanes.length === 2) {
            wsPanes.splice(index, 1)
            wsPanes[0].x = 0
            wsPanes[0].y = 0
            wsPanes[0].width = root.width
            wsPanes[0].height = root.height
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        } else if (index === 0) {
            let scale = (wsPanes.length - 1) / (wsPanes.length - 2)
            wsPanes[2].x = wsPanes[1].x
            wsPanes[2].y = wsPanes[1].y
            wsPanes[2].width = wsPanes[1].width
            wsPanes[2].height *= scale
            wsPanes[1].x = wsPanes[0].x
            wsPanes[1].y = wsPanes[0].y
            wsPanes[1].width = wsPanes[0].width
            wsPanes[1].height = wsPanes[0].height
            for (let i = 3; i < wsPanes.length; ++i) {
                wsPanes[i].height *= scale
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
            }
            wsPanes.splice(index, 1)
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        } else {
            let scale = (wsPanes.length - 1) / (wsPanes.length - 2)
            wsPanes.splice(index, 1)
            wsPanes[1].y = 0
            wsPanes[1].height *= scale
            for (let i = 2; i < wsPanes.length; i++) {
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                wsPanes[i].height *= scale
            }
            if (removeSurfaceIf === 1) {
                Helper.closeSurface(surfaceItem.shellSurface.surface)
                deleteFlag.push(1)
            } else if (removeSurfaceIf === 0) surfaceItem.visible = false
        }
        workspaceManager.syncWsPanes("removePane")
    }


    function swapPane() {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        if (selectSurfaceItem1 === null || selectSurfaceItem2 === null) {
            console.info(tCategory, "You should choose two pane before swap them.")
            return
        }

        let activeSurfaceItem = selectSurfaceItem1
        let targetSurfaceItem = selectSurfaceItem2
        let index1 = wsPanes.indexOf(activeSurfaceItem)
        let index2 = wsPanes.indexOf(targetSurfaceItem)

        if (index1 === index2) {
            console.info(tCategory, "You should choose two different panes to swap.")
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

        if (index1 === 0 || index2 === 0) {
            let tempXPos = activeSurfaceItem.x
            let tempYPos = activeSurfaceItem.y
            let tempWidth = activeSurfaceItem.width
            let tempHeight = activeSurfaceItem.height
            activeSurfaceItem.x = targetSurfaceItem.x
            activeSurfaceItem.y = targetSurfaceItem.y
            activeSurfaceItem.width = targetSurfaceItem.width
            activeSurfaceItem.height = targetSurfaceItem.height
            targetSurfaceItem.x = tempXPos
            targetSurfaceItem.y = tempYPos
            targetSurfaceItem.width = tempWidth
            targetSurfaceItem.height = tempHeight
        } else {
            let delta = activeSurfaceItem.height - targetSurfaceItem.height

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
    }

    function reLayout(surfaceItem) {
        let wsPanes = workspaceManager.wsPanesById.get(currentWsId)
        let index = wsPanes.indexOf(surfaceItem)
        if (wsPanes.length === 1) {
            wsPanes.splice(index, 1)
        } else if (wsPanes.length === 2) {
            wsPanes.splice(index, 1)
            wsPanes[0].x = 0
            wsPanes[0].y = 0
            wsPanes[0].width = root.width
            wsPanes[0].height = root.height
        } else if (index === 0) {
            let scale = (wsPanes.length - 1) / (wsPanes.length - 2)
            wsPanes[2].x = wsPanes[1].x
            wsPanes[2].y = wsPanes[1].y
            wsPanes[2].width = wsPanes[1].width
            wsPanes[2].height *= scale
            wsPanes[1].x = wsPanes[0].x
            wsPanes[1].y = wsPanes[0].y
            wsPanes[1].width = wsPanes[0].width
            wsPanes[1].height = wsPanes[0].height
            for (let i = 3; i < wsPanes.length; ++i) {
                wsPanes[i].height *= scale
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
            }
            wsPanes.splice(index, 1)
        } else {
            let scale = (wsPanes.length - 1) / (wsPanes.length - 2)

            wsPanes.splice(index, 1)
            wsPanes[1].y = 0
            wsPanes[1].height *= scale
            for (let i = 2; i < wsPanes.length; i++) {
                wsPanes[i].y = wsPanes[i-1].y + wsPanes[i-1].height
                wsPanes[i].height *= scale
            }
        }
        workspaceManager.syncWsPanes("reLayout", surfaceItem)
    }
}
