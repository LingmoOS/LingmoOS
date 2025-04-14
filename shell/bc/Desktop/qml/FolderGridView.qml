/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Lingmo.FileManager 1.0
import Lingmo.DragDrop 1.0 as DragDrop
import LingmoUI 1.0 as LingmoUI

GridView {
    id: control

    objectName: "FolderGridView"

    property bool isDesktopView: false

    property Item rubberBand: null
    property Item hoveredItem: null
    property Item pressedItem: null

    property alias positions: positioner.positions
    property alias positioner: positioner

    property int verticalDropHitscanOffset: 0

    property int pressX: -1
    property int pressY: -1

    property int dragX: -1
    property int dragY: -1

    property bool ctrlPressed: false
    property bool shiftPressed: false

    property variant cPress: null
    property Item editor: null
    property int anchorIndex: 0

    property var iconSize: settings.gridIconSize
    property var maximumIconSize: settings.maximumIconSize
    property var minimumIconSize: 22 //settings.minimumIconSize

    property variant cachedRectangleSelection: null

    property bool scrollLeft: false
    property bool scrollRight: false
    property bool scrollUp: false
    property bool scrollDown: false

    signal keyPress(var event)

    cacheBuffer: Math.max(0, control.height * 1.5)
    reuseItems: true

    onIconSizeChanged: {
        // 图标大小改变时需要重置状态，否则选中定位不准确
        positioner.reset()
    }

    onCountChanged: {
        positioner.reset()
    }

    function effectiveNavDirection(flow, layoutDirection, direction) {
        if (direction === Qt.LeftArrow) {
            if (flow === GridView.FlowLeftToRight) {
                if (layoutDirection === Qt.LeftToRight) {
                    return Qt.LeftArrow;
                } else {
                    return Qt.RightArrow;
                }
            } else {
                if (layoutDirection === Qt.LeftToRight) {
                    return Qt.UpArrow;
                } else {
                    return Qt.DownArrow;
                }
            }
        } else if (direction === Qt.RightArrow) {
            if (flow === GridView.FlowLeftToRight) {
                if (layoutDirection === Qt.LeftToRight) {
                    return Qt.RightArrow;
                } else {
                    return Qt.LeftArrow;
                }
            } else {
                if (layoutDirection === Qt.LeftToRight) {
                    return Qt.DownArrow;
                } else {
                    return Qt.UpArrow;
                }
            }
        } else if (direction === Qt.UpArrow) {
            if (flow === GridView.FlowLeftToRight) {
                return Qt.UpArrow;
            } else {
                return Qt.LeftArrow;
            }
        } else if (direction === Qt.DownArrow) {
            if (flow === GridView.FlowLeftToRight) {
                return Qt.DownArrow;
            } else {
                return Qt.RightArrow
            }
        }
    }

    function rename() {
        if (control.currentIndex != -1) {
            var renameAction = control.model.action("rename")
            if (renameAction && !renameAction.enabled)
                return

            if (!editor)
                editor = editorComponent.createObject(control)

            editor.targetItem = control.currentItem
        }
    }

    function cancelRename() {
        if (editor) {
            editor.cancel()
            editor.targetItem = null;
        }
    }

    function reset() {
        currentIndex = -1
        anchorIndex = 0
        cancelRename()
        hoveredItem = null
        pressedItem = null
        cPress = null
    }

    function drop(target, event, pos) {
        var dropPos = mapToItem(control.contentItem, pos.x, pos.y)
        var dropIndex = control.indexAt(dropPos.x, dropPos.y)
        var dragPos = mapToItem(control.contentItem, control.dragX, control.dragY)
        var dragIndex = control.indexAt(dragPos.x, dragPos.y)

        if (control.dragX === -1 || dragIndex !== dropIndex) {
            dirModel.drop(control, event, dropItemAt(dropPos))
        }
    }

    function dropItemAt(pos) {
        var item = control.itemAt(pos.x, pos.y)

        if (item) {
            if (item.blank) {
                return -1
            }

            var hOffset = Math.abs(Math.min(control.contentX, control.originX))
            var hPos = mapToItem(item, pos.x + hOffset, pos.y)

            if ((hPos.x < 0 || hPos.y < 0 || hPos.x > item.width || hPos.y > item.height)) {
                return -1
            } else {
                return positioner.map(item.index)
            }
        }

        return -1
    }

    highlightMoveDuration: 0
    keyNavigationEnabled : true
    keyNavigationWraps : true
    Keys.onPressed: {
        control.keyPress(event)

        if (event.key === Qt.Key_Control) {
            ctrlPressed = true
        } else if (event.key === Qt.Key_Shift) {
            shiftPressed = true

            if (currentIndex != -1)
                anchorIndex = currentIndex
        } else if (event.key === Qt.Key_Equal && event.modifiers & Qt.ControlModifier) {
            control.increaseIconSize()
        } else if (event.key === Qt.Key_Minus && event.modifiers & Qt.ControlModifier) {
            control.decreaseIconSize()
        }
    }
    Keys.onReleased: {
        if (event.key === Qt.Key_Control) {
            ctrlPressed = false
        } else if (event.key === Qt.Key_Shift) {
            shiftPressed = false
            anchorIndex = 0
        }
    }
    Keys.onEscapePressed: {
        if (!editor || !editor.targetItem) {
            dirModel.clearSelection()
            event.accepted = false
        }
    }
    Keys.onUpPressed: {
        if (!editor || !editor.targetItem) {
            var newIndex = positioner.nearestItem(currentIndex,
                                                  effectiveNavDirection(control.flow, control.effectiveLayoutDirection, Qt.UpArrow))
            if (newIndex !== -1) {
                currentIndex = newIndex
                updateSelection(event.modifiers)
            }
        }
    }
    Keys.onDownPressed: {
        if (!editor || !editor.targetItem) {
            var newIndex = positioner.nearestItem(currentIndex,
                                                  effectiveNavDirection(control.flow, control.effectiveLayoutDirection, Qt.DownArrow))
            if (newIndex !== -1) {
                currentIndex = newIndex
                updateSelection(event.modifiers)
            }
        }
    }
    Keys.onLeftPressed: {
        if (!editor || !editor.targetItem) {
            var newIndex = positioner.nearestItem(currentIndex,
                                                  effectiveNavDirection(control.flow, control.effectiveLayoutDirection, Qt.LeftArrow))
            if (newIndex !== -1) {
                currentIndex = newIndex
                updateSelection(event.modifiers)
            }
        }
    }
    Keys.onRightPressed: {
        if (!editor || !editor.targetItem) {
            var newIndex = positioner.nearestItem(currentIndex,
                                                  effectiveNavDirection(control.flow, control.effectiveLayoutDirection, Qt.RightArrow))
            if (newIndex !== -1) {
                currentIndex = newIndex
                updateSelection(event.modifiers)
            }
        }
    }

    cellHeight: {
        var iconHeight = iconSize + (LingmoUI.Units.fontMetrics.height * 2) + LingmoUI.Units.largeSpacing * 2
        if (isDesktopView) {
            var extraHeight = calcExtraSpacing(iconHeight, control.height - topMargin - bottomMargin)
            return iconHeight + extraHeight
        }
        return iconHeight
    }

    cellWidth: {
        var iconWidth = iconSize + LingmoUI.Units.largeSpacing * 4
        var extraWidth = calcExtraSpacing(iconWidth, control.width - leftMargin - rightMargin)
        return iconWidth + extraWidth
    }

    clip: true
    currentIndex: -1
    ScrollBar.vertical: ScrollBar { }

    LingmoUI.WheelHandler {
        target: control
    }

    onPressXChanged: {
        cPress = mapToItem(control.contentItem, pressX, pressY)
    }

    onPressYChanged: {
        cPress = mapToItem(control.contentItem, pressX, pressY)
    }

    onContentXChanged: {
        cancelRename()
    }

    onContentYChanged: {
        cancelRename()
    }

    onCachedRectangleSelectionChanged: {
        if (cachedRectangleSelection === null)
            return

        if (cachedRectangleSelection.length)
            control.currentIndex[0]

        dirModel.updateSelection(cachedRectangleSelection, control.ctrlPressed)
    }

    Positioner {
        id: positioner
        enabled: true
        folderModel: dirModel
        perStripe: Math.floor(((control.flow == GridView.FlowLeftToRight)
            ? control.width : control.height) / ((control.flow == GridView.FlowLeftToRight)
            ? control.cellWidth : control.cellHeight))
    }

    DragDrop.DropArea {
        anchors.fill: parent

        onDrop: {
            control.drop(control, event, mapToItem(control, event.x, event.y))
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        propagateComposedEvents: true
        preventStealing: true
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        hoverEnabled: true
        enabled: true
        z: -1

        onPressed: {
            control.forceActiveFocus()

            if (control.editor && childAt(mouse.x, mouse.y) !== control.editor)
                control.editor.commit()

            pressX = mouse.x
            pressY = mouse.y

            // 如果找到 hoveredItem 则点击后设置成为 pressedItem
            if (hoveredItem) {
                pressedItem = hoveredItem

                // 这里更改 currentIndex 会影响到范围选择
                if (!control.shiftPressed)
                    currentIndex = pressedItem.index

                // Shift 处理, 选择区域
                if (control.shiftPressed && control.currentIndex !== -1) {
                    dirModel.setRangeSelected(control.anchorIndex, hoveredItem.index)
                } else {
                    // Ctrl 处理
                    if (!control.ctrlPressed && !dirModel.isSelected(hoveredItem.index)) {
                        dirModel.clearSelection()
                    }

                    // Item 选择
                    if (control.ctrlPressed) {
                        dirModel.toggleSelected(hoveredItem.index)
                    } else {
                        dirModel.setSelected(hoveredItem.index)
                    }
                }

                // 弹出 Item 菜单
                if (mouse.buttons & Qt.RightButton) {
                    clearPressState()
                    dirModel.openContextMenu(null, mouse.modifiers)
                    mouse.accepted = true
                }
            } else {
                // 处理空白区域点击
                if (!control.ctrlPressed) {
                    control.currentIndex = -1
                    dirModel.clearSelection()
                }

                // 弹出文件夹菜单
                if (mouse.buttons & Qt.RightButton) {
                    clearPressState()
                    dirModel.openContextMenu(null, mouse.modifiers)
                    mouse.accepted = true
                }
            }
        }

        onPositionChanged: {
            control.ctrlPressed = (mouse.modifiers & Qt.ControlModifier)
            control.shiftPressed = (mouse.modifiers & Qt.ShiftModifier)

            var index = control.indexAt(mouse.x + control.contentX,
                                        mouse.y + control.contentY)
            var indexItem = control.itemAtIndex(index)

            // Set hoveredItem.
            if (indexItem) {
                var fPos = mapToItem(indexItem.background, mouse.x, mouse.y)

                if (fPos.x < 0 || fPos.y < 0
                        || fPos.x > indexItem.background.width
                        || fPos.y > indexItem.background.height) {
                    control.hoveredItem = null
                } else {
                    control.hoveredItem = indexItem
                }
            } else {
                control.hoveredItem = null
            }

            // Update rubberband geometry.
            if (control.rubberBand) {
                var cPos = mapToItem(control.contentItem, mouse.x, mouse.y)
                var leftEdge = Math.min(control.contentX, control.originX)
                var rB = control.rubberBand

                if (cPos.x < cPress.x) {
                    rB.x = Math.max(leftEdge, cPos.x)
                    rB.width = Math.abs(rB.x - cPress.x)
                } else {
                    rB.x = cPress.x
                    const ceil = Math.max(control.width, control.contentItem.width) + leftEdge
                    rB.width = Math.min(ceil - rB.x, Math.abs(rB.x - cPos.x))
                }

                if (cPos.y < cPress.y) {
                    rB.y = Math.max(0, cPos.y)
                    rB.height = Math.abs(rB.y - cPress.y)
                } else {
                    rB.y = cPress.y
                    const ceil = Math.max(control.height, control.contentItem.height)
                    rB.height = Math.min(ceil - rB.y, Math.abs(rB.y - cPos.y))
                }

                // Ensure rubberband is at least 1px in size or else it will become
                // invisible and not match any items.
                rB.width = Math.max(1, rB.width)
                rB.height = Math.max(1, rB.height)

                control.rectangleSelect(rB.x, rB.y, rB.width, rB.height)

                return
            }

            if (pressX != -1) {
                if (pressedItem != null && dirModel.isSelected(pressedItem.index)) {
                    control.dragX = mouse.x
                    control.dragY = mouse.y
                    control.verticalDropHitscanOffset = pressedItem.y + (pressedItem.height / 2)
                    dirModel.dragSelected(mouse.x, mouse.y)
                    control.dragX = -1
                    control.dragY = -1
                    clearPressState()
                } else {
                    if (control.editor && control.editor.targetItem)
                        return

                    dirModel.pinSelection()
                    control.rubberBand = rubberBandObject.createObject(control.contentItem, {x: cPress.x, y: cPress.y})
                    control.interactive = false
                }
            }
        }

        onClicked: {
            clearPressState()

            if (mouse.buttons & Qt.RightButton) {
                dirModel.openContextMenu(null, mouse.modifiers)
            }
        }

        onDoubleClicked: {
            if (mouse.button === Qt.LeftButton && control.pressedItem)
                dirModel.openSelected()
        }

        onReleased: {
            // 当选择多个文件的时候，在这选择里的文件中点击
            if (pressedItem != null &&
                    !control.rubberBand &&
                    !control.shiftPressed &&
                    !control.ctrlPressed &&
                    !dirModel.dragging) {
                dirModel.clearSelection()
                dirModel.setSelected(pressedItem.index)
            }

            dirModel.updateSelectedItemsSize()

            pressCanceled()
        }

        onCanceled: pressCanceled()

        onWheel: {
            if (wheel.modifiers & Qt.ControlModifier) {
                if (wheel.angleDelta.y > 0)
                    control.increaseIconSize()
                else
                    control.decreaseIconSize()
            } else {
                wheel.accepted = false
            }
        }
    }

    function clearPressState() {
        pressedItem = null
        pressX = -1
        pressY = -1
    }

    function pressCanceled() {
        if (control.rubberBand) {
            control.rubberBand.close()
            control.rubberBand = null

            control.interactive = true
            control.cachedRectangleSelection = null
            dirModel.unpinSelection()
        }

        clearPressState()
    }

    function rectangleSelect(x, y, width, height) {
        var rows = (control.flow === GridView.FlowLeftToRight)
        var axis = rows ? control.width : control.height
        var step = rows ? cellWidth : cellHeight
        var perStripe = Math.floor(axis / step)
        var stripes = Math.ceil(control.count / perStripe)
        var cWidth = control.cellWidth - (2 * LingmoUI.Units.smallSpacing)
        var cHeight = control.cellHeight - (2 * LingmoUI.Units.smallSpacing)
        var midWidth = control.cellWidth / 2
        var midHeight = control.cellHeight / 2
        var indices = []

        for (var s = 0; s < stripes; s++) {
            for (var i = 0; i < perStripe; i++) {
                var index = (s * perStripe) + i

                if (index >= control.count) {
                    break
                }

                if (dirModel.isBlank(index)) {
                    continue
                }

                var itemX = ((rows ? i : s) * control.cellWidth)
                var itemY = ((rows ? s : i) * control.cellHeight)

                if (control.effectiveLayoutDirection === Qt.RightToLeft) {
                    itemX -= (rows ? control.contentX : control.originX)
                    itemX += cWidth
                    itemX = (rows ? control.width : control.contentItem.width) - itemX
                }

                // Check if the rubberband intersects this cell first to avoid doing more
                // expensive work.
                if (control.rubberBand.intersects(Qt.rect(itemX + LingmoUI.Units.smallSpacing, itemY + LingmoUI.Units.smallSpacing,
                    cWidth, cHeight))) {
                    var item = control.contentItem.childAt(itemX + midWidth, itemY + midHeight)

                    // If this is a visible item, check for intersection with the actual
                    // icon or label rects for better feel.
                    if (item && item.iconArea) {
                        var iconRect = Qt.rect(itemX + item.iconArea.x, itemY + item.iconArea.y,
                            item.iconArea.width, item.iconArea.height)

                        if (control.rubberBand.intersects(iconRect)) {
                            indices.push(index)
                            continue
                        }

                        var labelRect = Qt.rect(itemX + item.labelArea.x, itemY + item.labelArea.y,
                            item.labelArea.width, item.labelArea.height)

                        if (control.rubberBand.intersects(labelRect)) {
                            indices.push(index)
                            continue
                        }
                    } else {
                        // Otherwise be content with the cell intersection.
                        indices.push(index)
                    }
                }
            }
        }

        control.cachedRectangleSelection = indices
    }

    function calcExtraSpacing(cellSize, containerSize) {
        var availableColumns = Math.floor(containerSize / cellSize)
        var extraSpacing = 0
        if (availableColumns > 0) {
            var allColumnSize = availableColumns * cellSize
            var extraSpace = Math.max(containerSize - allColumnSize, 0)
            extraSpacing = extraSpace / availableColumns
        }
        return Math.floor(extraSpacing)
    }

    function increaseIconSize() {
        if (iconSize >= control.maximumIconSize) {
            iconSize = control.maximumIconSize
            return
        }

        iconSize += (iconSize * 0.1)
    }

    function decreaseIconSize() {
        if (iconSize <= control.minimumIconSize) {
            iconSize = control.minimumIconSize
            return
        }

        iconSize -= (iconSize * 0.1)
    }

    function updateSelection(modifier) {
        if (modifier & Qt.ShiftModifier) {
            dirModel.setRangeSelected(anchorIndex, currentIndex)
        } else {
            dirModel.clearSelection()
            dirModel.setSelected(currentIndex)
        }
    }

    Component {
        id: editorComponent

        TextField {
            id: _editor
            visible: false
            wrapMode: TextEdit.Wrap
            horizontalAlignment: TextEdit.AlignHCenter
            topPadding: LingmoUI.Units.smallSpacing
            bottomPadding: LingmoUI.Units.smallSpacing
            z: 999

            property Item targetItem: null

            onTargetItemChanged: {
                if (targetItem != null) {
                    var pos = control.mapFromItem(targetItem, targetItem.labelArea.x, targetItem.labelArea.y)
                    width = targetItem.width - LingmoUI.Units.smallSpacing
                    height = targetItem.labelArea.paintedHeight + LingmoUI.Units.largeSpacing * 2
                    x = targetItem.x + Math.abs(Math.min(control.contentX, control.originX))
                    y = pos.y - LingmoUI.Units.smallSpacing
                    text = targetItem.labelArea.text
                    targetItem.labelArea.visible = false
                    _editor.select(0, dirModel.fileExtensionBoundary(targetItem.index))
                    visible = true
                    control.interactive = false
                } else {
                    x = 0
                    y = 0
                    width = 0
                    height = 0
                    visible = false
                    control.interactive = true
                }
            }

            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    commit()
                    event.accepted = true
                    break
                case Qt.Key_Escape:
                    cancel()
                    event.accepted = true
                    break
                }
            }

            onVisibleChanged: {
                if (visible)
                    _editor.forceActiveFocus()
                else
                    control.forceActiveFocus()
            }

            function commit() {
                if (targetItem) {
                    targetItem.labelArea.visible = true
                    dirModel.rename(targetItem.index, text)
                    control.currentIndex = targetItem.index
                    targetItem = null

                    control.editor.destroy()
                }
            }

            function cancel() {
                if (targetItem) {
                    targetItem.labelArea.visible = true
                    control.currentIndex = targetItem.index
                    targetItem = null

                    control.editor.destroy()
                }
            }
        }
    }
}
