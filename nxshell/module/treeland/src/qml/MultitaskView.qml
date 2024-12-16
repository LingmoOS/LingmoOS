// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identif ier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Utils
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

Item {
    id: root
    required property int currentWorkspaceId
    required property var setCurrentWorkspaceId
    property ListModel model: QmlHelper.workspaceManager.workspacesById.get(QmlHelper.workspaceManager.layoutOrder.get(currentWorkspaceId).wsid).surfaces
    property int current: -1

    function exit(surfaceItem) {
        if (surfaceItem)
            Helper.activatedSurface = surfaceItem.shellSurface
        else if (current >= 0 && current < model.count)
            Helper.activatedSurface = model.get(current).item.shellSurface
        visible = false
    }

    QtObject {
        id: dragManager
        property Item item  // current dragged item
        property var accept // accept callback func
    }

    Item {
        id: outputsPlacementItem
        Repeater {
            model: Helper.outputLayout.outputs
            MouseArea {
                id: outputPlacementItem
                function calcDisplayRect(item, output) {
                    // margins on this output
                    const margins = Helper.getOutputExclusiveMargins(output)
                    const coord = parent.mapFromItem(item, 0, 0)
                    // global pos before culling zones
                    const origin = Qt.rect(coord.x, coord.y, item.width, item.height)
                    return Qt.rect(origin.x + margins.left, origin.y + margins.top,
                        origin.width - margins.left - margins.right, origin.height - margins.top - margins.bottom)
                }
                property rect displayRect: root.visible ? calcDisplayRect(modelData, modelData.output) : Qt.rect(0, 0, 0, 0)
                x: displayRect.x
                y: displayRect.y
                width: displayRect.width
                height: displayRect.height

                onClicked: root.exit()

                ColumnLayout {
                    anchors.fill: parent
                    DelegateModel {
                        id: visualModel
                        model: workspaceManager.layoutOrder
                        delegate: Rectangle {
                            id: wsThumbItem
                            required property int wsid
                            required property int index
                            height: workspacesList.height
                            width: height * outputPlacementItem.width / outputPlacementItem.height
                            border.width: workspaceManager.workspacesById.get(wsid).isCurrentWorkspace ? 2 : 0
                            border.color: "blue"
                            color: "transparent"
                            Item {
                                anchors {
                                    fill: parent
                                    margins: 10
                                }
                                clip: true
                                ShaderEffectSource {
                                    sourceItem: activeOutputDelegate
                                    anchors.fill: parent
                                }
                                ShaderEffectSource {
                                    sourceItem: workspaceManager.workspacesById.get(wsid)
                                    sourceRect: outputPlacementItem.displayRect
                                    anchors.fill: parent
                                }
                                HoverHandler {
                                    id: hvrhdlr
                                    enabled: !drg.active
                                    onHoveredChanged: if (hovered) {
                                        if (dragManager.item) {
                                            if (dragManager.item.source) {  // is dragging surface
                                                dragManager.accept = () => {
                                                    dragManager.item.source.workspaceId = wsid
                                                }
                                            } else {    // is dragging workspace
                                                dragManager.accept = () => {
                                                    const draggedItem = dragManager.item
                                                    const draggedWs = QmlHelper.workspaceManager.workspacesById.get(draggedItem.wsid)
                                                    const destIndex = draggedItem.DelegateModel.itemsIndex
                                                    QmlHelper.workspaceManager.layoutOrder.move(draggedWs.workspaceRelativeId, destIndex, 1)
                                                    // make curWorkspaceId follow the workspace
                                                    root.setCurrentWorkspaceId(destIndex)
                                                    // reset dragged item's align
                                                    visualModel.items.move(destIndex, destIndex)
                                                    draggedItem.y = draggedItem.initialState.y  // y is not set by layout?
                                                }
                                                visualModel.items.move(dragManager.item.DelegateModel.itemsIndex, wsThumbItem.DelegateModel.itemsIndex)
                                            }
                                        }
                                    } else {
                                        if (dragManager.item?.source) // is dragging surface, workspace always lose hover
                                            dragManager.accept = null
                                    }
                                }
                                DelegateModel.inPersistedItems: true
                                TapHandler {
                                    id: taphdlr
                                    onTapped: {
                                        if (root.currentWorkspaceId === index)
                                            multitaskView.active = false
                                        else
                                            root.setCurrentWorkspaceId(index)
                                    }
                                }
                                Rectangle {
                                    anchors.fill: parent
                                    color: hvrhdlr.hovered ? Qt.rgba(0, 0, 0, .2) : Qt.rgba(0, 0, 0, 0)
                                    Text {
                                        anchors.centerIn: parent
                                        color: "white"
                                        text: `No.${index}`
                                    }
                                }
                            }

                            D.RoundButton {
                                id: wsDestroyBtn
                                icon.name: "window_close"
                                icon.width: 20
                                icon.height: 20
                                height: 24
                                width: height
                                visible: (workspaceManager.layoutOrder.count > 1)
                                    && hvrhdlr.hovered
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                }
                                Item {
                                    id: control
                                    property D.Palette textColor: DS.Style.button.text
                                }
                                textColor: control.textColor
                                background: Rectangle {
                                    anchors.fill: parent
                                    color: "white"
                                    radius: parent.height / 2
                                }
                                onClicked: {
                                    workspaceManager.destroyWs(parent.index)
                                    root.model = QmlHelper.workspaceManager.workspacesById.get(QmlHelper.workspaceManager.layoutOrder.get(currentWorkspaceId).wsid).surfaces
                                }
                            }

                            property var initialState
                            DragHandler {
                                id: drg
                                onActiveChanged: if (active) {
                                    dragManager.item = parent
                                    initialState = {x: parent.x, y: parent.y}
                                } else {
                                    if (dragManager.accept) {
                                        dragManager.accept()
                                    } else {
                                        parent.x = initialState.x
                                        parent.y = initialState.y
                                        visualModel.items.move(parent.DelegateModel.itemsIndex, parent.index)
                                    }
                                    dragManager.item = null
                                }
                            }
                        }
                    }
                    Item {
                        id: workspacesListContainer
                        Layout.preferredHeight: outputPlacementItem.height * .2
                        Layout.preferredWidth: parent.width
                        Layout.alignment: Qt.AlignHCenter
                        ListView {
                            id: workspacesList
                            orientation: ListView.Horizontal
                            model: visualModel
                            height: parent.height
                            width: Math.min(parent.width,
                                model.count * height * outputPlacementItem.width / outputPlacementItem.height)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        D.RoundButton {
                            id: wsCreateBtn
                            anchors {
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                                margins: 20
                            }
                            height: 80
                            width: 80
                            icon.name: "list_add"
                            icon.height: height
                            icon.width: width
                            background: Rectangle {
                                color: Qt.rgba(255, 255, 255, .4)
                                anchors.fill: parent
                                radius: 20
                            }
                            onClicked: {
                                workspaceManager.createWs()
                            }
                        }
                    }
                    Loader {
                        id: surfacesGridView
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.margins: 30

                        active: false
                        Component.onCompleted: {
                            // must after wslist's height stablized, so that surfaces animation is initialized correctly
                            active = true
                        }

                        sourceComponent: Component {
                            Item {
                                anchors.fill: parent
                                FilterProxyModel {
                                    id: outputProxy
                                    sourceModel: root.model
                                    property bool initialized: false
                                    filterAcceptsRow: (d) => {
                                        const item = d.item
                                        if (!(item instanceof SurfaceItem))
                                            return false
                                        return item.waylandSurface.surface.primaryOutput === modelData.output
                                    }
                                    Component.onCompleted: {
                                        initialized = true  // TODO better initialize timing
                                        invalidate()
                                        grid.calcLayout()
                                    }
                                    onSourceModelChanged: {
                                        invalidate()
                                        if (initialized) grid.calcLayout()
                                    }
                                }

                                EQHGrid {
                                    id: grid
                                    anchors.fill: parent
                                    model: outputProxy
                                    minH: 100
                                    maxH: parent.height
                                    maxW: parent.width
                                    availH: parent.height
                                    availW: parent.width
                                    getRatio: (d) => d.item.width / d.item.height
                                    enterAnimationEnabled: true
                                    delegate: Item {
                                            property SurfaceItem source: modelData.item

                                            property real ratio: source.width / source.height
                                            onRatioChanged: {
                                                grid.calcLayout()
                                            }

                                            property var initialState
                                            property real animRatio: 1
                                            function conv(y, item = parent) { // convert to outputPlacementItem's coord
                                                return mapToItem(outputPlacementItem, mapFromItem(item, 0, y)).y
                                            }
                                            onYChanged: {
                                                // ori * ratio(y=destY) = destw, ori * ratio(y=oriY) = ori
                                                const destW = 100
                                                const destY = conv(workspacesList.height, workspacesList)
                                                const deltY = Math.max(conv(Math.min(y, initialState.y)) - destY, 0)
                                                const fullY = conv(0) - destY
                                                animRatio = ( (( fullY - deltY) / fullY) * (destW - initialState.width) + initialState.width) / initialState.width
                                            }

                                            width: displayWidth * animRatio
                                            height: width * source.height / source.width
                                            clip: true
                                            z: drg.active ? 1 : 0   // dragged item should float
                                            property bool highlighted: dragManager.item == this || (!dragManager.item && hvhdlr.hovered)
                                            HoverHandler {
                                                id: hvhdlr
                                            }
                                            TapHandler {
                                                onTapped: root.exit(source)
                                            }
                                            DragHandler {
                                                id: drg
                                                property var curState
                                                onActiveChanged: if (active) {
                                                    dragManager.item = parent
                                                    initialState = {x: parent.x, y: parent.y, width: parent.width}
                                                } else {
                                                    if (dragManager.accept) {
                                                        dragManager.accept()
                                                    } else {
                                                        parent.x = initialState.x
                                                        parent.y = initialState.y
                                                        parent.animRatio = 1
                                                    }
                                                    dragManager.item = null
                                                }
                                            }
                                            Rectangle {
                                                anchors.fill: parent
                                                color: "transparent"
                                                border.width: highlighted ? 2 : 0
                                                border.color: "blue"
                                                radius: 8
                                            }
                                            ShaderEffectSource {
                                                anchors {
                                                    fill: parent
                                                    margins: 3
                                                }
                                                live: true
                                                // no hidesource, may conflict with workspace thumb
                                                smooth: true
                                                sourceItem: source
                                            }
                                            Control {
                                                id: titleBox
                                                anchors {
                                                    bottom: parent.bottom
                                                    horizontalCenter: parent.horizontalCenter
                                                    margins: 10
                                                }
                                                width: Math.min(implicitContentWidth + 2 * padding, parent.width * .7)
                                                padding: 10
                                                visible: highlighted

                                                contentItem: Text {
                                                    text: source.waylandSurface.title
                                                    elide: Qt.ElideRight
                                                }
                                                background: Rectangle {
                                                    color: Qt.rgba(255, 255, 255, .2)
                                                    radius: 5
                                                }
                                            }
                                        }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
