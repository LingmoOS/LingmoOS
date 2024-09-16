// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWayland.Compositor
import Qt.labs.platform 1.1 as LP
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.private 1.0 as DP

import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0
import org.deepin.ds.dock.tray 1.0 as DDT

AppletItemButton {
    id: root
    property alias inputEventsEnabled: surfaceItem.inputEventsEnabled

    property size visualSize: isHorizontal ? Qt.size(pluginItem.implicitWidth, Math.min(itemHeight, pluginItem.implicitHeight))
                                           : Qt.size(Math.min(itemWidth, pluginItem.implicitWidth), pluginItem.implicitHeight)

    readonly property int itemWidth: isHorizontal ? 0 : DDT.TrayItemPositionManager.dockHeight
    readonly property int itemHeight: isHorizontal ? DDT.TrayItemPositionManager.dockHeight : 0

    required property bool itemVisible
    property bool dragable: true

    padding: 0

    visible: !Drag.active
    hoverEnabled: inputEventsEnabled

    function updatePluginMargins()
    {
        pluginItem.plugin.margins = itemPadding
    }

    contentItem: Item {
        id: pluginItem
        property var plugin: DockCompositor.findSurface(model.surfaceId)
        implicitHeight: surfaceItem.height
        implicitWidth: surfaceItem.width

        property var itemGlobalPoint: {
            var a = pluginItem
            var x = 0, y = 0
            while(a.parent) {
                x += a.x
                y += a.y
                a = a.parent
            }

            return Qt.point(x, y)
        }
        
        property var itemGlobalPos: {
            var a = pluginItem
            var x = 0, y = 0

            if (a.Window.window && surfaceItem.visible) {
                while (a.parent) {
                    x += a.x
                    y += a.y
                    a = a.parent
                }
                x += pluginItem.Window.window.x
                y += pluginItem.Window.window.y

            }

            return Qt.point(x, y)
        }

        HoverHandler {
            id: hoverHandler
            parent: surfaceItem
        }
        TapHandler {
            id: tapHandler
            parent: surfaceItem
        }

        ShellSurfaceItem {
            id: surfaceItem
            anchors.centerIn: parent
            shellSurface: pluginItem.plugin
        }

        Component.onCompleted: {
            if (!pluginItem.plugin || !itemVisible)
                return
            updatePluginMargins()
            pluginItem.plugin.updatePluginGeometry(Qt.rect(pluginItem.itemGlobalPoint.x, pluginItem.itemGlobalPoint.y, 0, 0))
            pluginItem.plugin.setGlobalPos(pluginItem.itemGlobalPos)
        }

        Timer {
            id: updatePluginItemGeometryTimer
            interval: 200
            running: false
            repeat: false
            onTriggered: {
                if (!pluginItem.plugin || !itemVisible)
                    return
                updatePluginMargins()
                if (pluginItem.itemGlobalPoint.x >= 0 && pluginItem.itemGlobalPoint.y >= 0) {
                    pluginItem.plugin.updatePluginGeometry(Qt.rect(pluginItem.itemGlobalPoint.x, pluginItem.itemGlobalPoint.y, 0, 0))
                }
            }
        }

        Timer {
            id: updatePluginItemPosTimer
            interval: 200
            running: false
            repeat: false
            onTriggered: {
                if (!pluginItem.plugin || !itemVisible)
                    return
                pluginItem.plugin.setGlobalPos(pluginItem.itemGlobalPos)
            }
        }

        onItemGlobalPointChanged: {
            updatePluginItemGeometryTimer.start()
        }

        onItemGlobalPosChanged: {
            updatePluginItemPosTimer.start()
        }

        onVisibleChanged: {
            if (!pluginItem.plugin || !itemVisible)
                return
            updatePluginMargins()
            pluginItem.plugin.setGlobalPos(pluginItem.itemGlobalPos)
        }
    }

    D.ColorSelector.hovered: root.inputEventsEnabled && (pluginItem.plugin && pluginItem.plugin.isItemActive || hoverHandler.hovered)
    D.ColorSelector.pressed: tapHandler.pressed

    property Component overlayWindow: QuickDragWindow {
        height: root.visualSize.height
        width: root.visualSize.width

        Loader {
            height: parent.height
            width: parent.width
            active: root.DQuickDrag.isDragging
            sourceComponent: ShellSurfaceItem {
                anchors.centerIn: parent
                shellSurface: pluginItem.plugin
            }
        }
    }

    Drag.dragType: Drag.Automatic
    DQuickDrag.overlay: overlayWindow
    DQuickDrag.active: Drag.active
    DQuickDrag.hotSpotScale: Qt.size(0.5, 1)
    Drag.mimeData: {
        "text/x-dde-shell-tray-dnd-surfaceId": model.surfaceId,
        "text/x-dde-shell-tray-dnd-sectionType": model.sectionType
    }
    Drag.supportedActions: Qt.MoveAction
    Drag.onActiveChanged: {
        DDT.TraySortOrderModel.actionsAlwaysVisible = Drag.active
        if (!Drag.active) {
            // reset position on drop
            Qt.callLater(() => { x = 0; y = 0; });
        }
    }

    DragHandler {
        id: dragHandler
        enabled: dragable
        // To avoid being continuously active in a short period of time
        onActiveChanged: {
            Qt.callLater(function() { root.Drag.active = dragHandler.active })
        }
    }
}
