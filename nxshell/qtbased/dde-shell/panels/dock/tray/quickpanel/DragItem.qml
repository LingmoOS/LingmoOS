// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window

import org.deepin.ds 1.0
import org.deepin.dtk 1.0
import org.deepin.ds.dock.tray 1.0
import org.deepin.ds.dock.tray.quickpanel 1.0

Item {
    id: root

    required property Item dragItem
    required property string dragTextData
    required property var fallbackDragImage
    property bool enabledDrag

    Component.onCompleted: {
        dragItem.Drag.mimeData = Qt.binding(function () {
            return {
                "text/x-dde-shell-tray-dnd-surfaceId": root.dragTextData
            }
        })
        dragItem.Drag.dragType = Drag.Automatic
        dragItem.Drag.supportedActions = Qt.CopyAction
        dragItem.DQuickDrag.hotSpotScale = Qt.binding(function () {
            return Qt.size(0.5, 0.5)
        })
        dragItem.DQuickDrag.active = Qt.binding(function () {
            return dragItem.Drag.active
        })
        dragItem.DQuickDrag.overlay = Qt.binding(function () {
            return overlayWindow
        })
    }

    property string draggingImage
    property size fallbackIconSize: Qt.size(16, 16)

    property Component overlayWindow: QuickDragWindow {
        property size dragItemSize
        property point startDragPoint
        property point currentDragPoint
        property int endDragPointArea
        property bool isFallbackIcon: {
            return height <= root.fallbackIconSize.height
        }

        startDragPoint: dragItem.DQuickDrag.startDragPoint
        currentDragPoint: dragItem.DQuickDrag.currentDragPoint
        dragItemSize: Qt.size(dragItem.width, dragItem.height)
        endDragPointArea: {
            return Panel.rootObject.y - root.fallbackIconSize.height * dragItem.DQuickDrag.hotSpotScale.height
        }

        // Height and position follow linear transformation, y = k * h + b.
        // and (y, h) exist two rules, (start.y, drag.h) and (end.y, 16)
        function getHeight() {
            if (currentDragPoint.y < startDragPoint.y)
                return dragItemSize.height
            if (currentDragPoint.y > endDragPointArea)
                return root.fallbackIconSize.height

            var k = (startDragPoint.y - endDragPointArea) * 1.0 / (dragItemSize.height - root.fallbackIconSize.height)
            var b = startDragPoint.y - k * dragItemSize.height * 1.0
            var h = (currentDragPoint.y * 1.0 - b) / k
            return h
        }

        // TODO: turn on this can cause DnD drag event get rejected, it's likely a dxcb bug.
        // DWindow.enabled: true
        // DWindow.enableBlurWindow: !isFallbackIcon
        // DWindow.shadowRadius: 0
        // DWindow.borderWidth: 0
        ColorSelector.family: Palette.CrystalColor

        height: getHeight()
        width: !isFallbackIcon ? (dragItemSize.width * 1.0 / dragItemSize.height) * height : root.fallbackIconSize.width

        Loader {
            active: !isFallbackIcon
            anchors.fill: parent

            sourceComponent: Image {
                source: root.draggingImage
            }
        }

        Loader {
            active: isFallbackIcon && root.dragItem.DQuickDrag.isDragging
            anchors.fill: parent
            sourceComponent: ShellSurfaceItemProxy {
                anchors.centerIn: parent
                shellSurface: root.fallbackDragImage
            }
        }
    }

    DragHandler {
        id: dragHandler
        enabled: enabledDrag
        dragThreshold: 5
        onActiveChanged: {
            if (active) {
                dragItem.grabToImage(function(result) {
                    console.log("grab to image", result.url)

                    draggingImage = result.url
                })
            }
            dragItem.Drag.active = active
        }
    }
}
