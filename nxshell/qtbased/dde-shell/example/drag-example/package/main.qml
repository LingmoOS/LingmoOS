// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQml
import QtQml.Models

import org.deepin.ds 1.0
import org.deepin.dtk 1.0

AppletItem {
    objectName: "appplet item"
    implicitWidth: gridview.width
    implicitHeight: gridview.height
    GridView {
        id: gridview
        width: 320
        height: 100
        cellWidth: 80
        cellHeight: 80

        model: DelegateModel {
            id: visualModel
            model: ListModel {
                id: colorModel
                ListElement { color: "blue" }
                ListElement { color: "green" }
                ListElement { color: "red" }
                ListElement { color: "yellow" }
            }
            delegate: Item {
                id: dragItem
                required property color color

                width: 80
                height: 80
                Rectangle {
                    anchors.fill: parent
                    color: dragItem.color
                }
                Drag.keys: [
                    "text/x-dde-dock-dnd-appid"
                ]
                Drag.mimeData: {
                    "text/plain": "Copied text"
                }
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.CopyAction
                // Drag.hotSpot.x: width / 2
                // Drag.hotSpot.y: height / 2
                DQuickDrag.hotSpotScale: Qt.size(0.5, 0.5)
                DQuickDrag.overlay: overlayWindow
                DQuickDrag.active: Drag.active

                property string fallbackDragImage: "computer-symbolic"
                property string draggingImage
                property size fallbackIconSize: Qt.size(16, 16)
                property Component overlayWindow: QuickDragWindow {
                    property size dragItemSize
                    property string iconName
                    property point startDragPoint
                    property point currentDragPoint
                    property int endDragPointArea
                    property bool isFallbackIcon: {
                        return height <= dragItem.fallbackIconSize.height
                    }

                    startDragPoint: dragItem.DQuickDrag.startDragPoint
                    currentDragPoint: dragItem.DQuickDrag.currentDragPoint
                    dragItemSize: Qt.size(dragItem.width, dragItem.height)
                    endDragPointArea: {
                        return Screen.height / 2 - dragItem.fallbackIconSize.height * dragItem.DQuickDrag.hotSpotScale.height
                    }
                    iconName: !isFallbackIcon ? dragItem.draggingImage : dragItem.fallbackDragImage

                    // Height and position follow linear transformation, y = k * h + b.
                    // and (y, h) exist two rules, (start.y, drag.h) and (end.y, 16)
                    function getHeight() {
                        if (currentDragPoint.y < startDragPoint.y)
                            return dragItemSize.height
                        if (currentDragPoint.y > endDragPointArea)
                            return dragItem.fallbackIconSize.height

                        var k = (startDragPoint.y - endDragPointArea) * 1.0 / (dragItemSize.height - dragItem.fallbackIconSize.height)
                        var b = startDragPoint.y - k * dragItemSize.height * 1.0
                        var h = (currentDragPoint.y * 1.0 - b) / k
                        return h
                    }

                    height: getHeight()
                    width: (dragItemSize.width * 1.0 / dragItemSize.height) * height

                    DciIcon {
                        id: iconView
                        anchors.fill: parent
                        sourceSize: Qt.size(width, height)
                        asynchronous: false
                        name: iconName
                    }
                }

                DragHandler {
                    onActiveChanged: {
                        if (active) {
                            dragItem.grabToImage(function(result) {
                                console.log("grab to image", result.url)
                                var local = "/tmp/" + result.url + ".png"
                                local = local.replace(/#/g, '-')
                                result.saveToFile(local)

                                dragItem.draggingImage = "file://" + local
                            })
                        }
                        dragItem.Drag.active = active
                    }
                }
            }
        }
    }
}
