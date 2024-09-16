// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor
import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0

Item {
    id: root

    property var surfaceAcceptor: function (surfaceId) {
        return true
    }
    property int toolTipVOffset: 0
    signal popupCreated(var surfaceId)

    PanelToolTipWindow {
        id: toolTipWindow
        onVisibleChanged: function (arg) {
            if (arg && menuWindow.visible)
                menuWindow.close()
        }
    }
    PanelToolTip {
        id: toolTip
        toolTipWindow: toolTipWindow

        property alias shellSurface: surfaceLayer.shellSurface
        ShellSurfaceItemProxy {
            id: surfaceLayer
            anchors.centerIn: parent
            autoClose: true
            onSurfaceDestroyed: function () {
                toolTip.close()
            }
        }
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: toolTip.toolTipWindow
        sizeFollowsWindow: false
    }

    PanelMenuWindow {
        id: menuWindow
        onVisibleChanged: function (arg) {
            if (arg && toolTipWindow.visible)
                toolTipWindow.close()
        }
        onUpdateGeometryFinished: function () {
            if (!menu.shellSurface)
                return
            menu.shellSurface.updatePluginGeometry(Qt.rect(menu.menuWindow.x, menu.menuWindow.y, 0, 0))
        }
    }
    PanelMenu {
        id: menu
        width: menuSurfaceLayer.width
        height: menuSurfaceLayer.height
        menuWindow: menuWindow

        property alias shellSurface: menuSurfaceLayer.shellSurface
        ShellSurfaceItemProxy {
            id: menuSurfaceLayer
            anchors.centerIn: parent
            autoClose: true
            onSurfaceDestroyed: function () {
                menu.close()
            }
        }
        Loader {
            active: menu.menuVisible
            sourceComponent: SurfaceSubPopup {
                objectName: "stashed's subPopup"
                transientParent: menuWindow
                surfaceAcceptor: function (surfaceId) {
                    if (root.surfaceAcceptor && !root.surfaceAcceptor(surfaceId))
                        return false
                    return true
                }
            }
        }
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: menu.menuWindow
        sizeFollowsWindow: false
    }

    Connections {
        target: DockCompositor
        function onPopupCreated(popupSurface) {
            let surfaceId = `${popupSurface.pluginId}::${popupSurface.itemKey}`
            if (surfaceAcceptor && !surfaceAcceptor(surfaceId))
                return

            popupCreated(surfaceId)

            if (popupSurface.popupType === Dock.TrayPopupTypeTooltip) {
                console.log(root.objectName, ": tooltip created", popupSurface.popupType, popupSurface.pluginId)

                toolTip.shellSurface = popupSurface
                toolTip.toolTipX = Qt.binding(function () {
                    return toolTip.shellSurface.x - toolTip.width / 2
                })
                toolTip.toolTipY = Qt.binding(function () {
                    return toolTip.shellSurface.y - toolTip.height - toolTipVOffset
                })
                toolTip.open()
            } else if (popupSurface.popupType === Dock.TrayPopupTypeMenu) {
                console.log(root.objectName, ": menu created", popupSurface.popupType, popupSurface.pluginId)

                menu.shellSurface = popupSurface
                menu.menuX = Qt.binding(function () {
                    return menu.shellSurface.x
                })
                menu.menuY = Qt.binding(function () {
                    return menu.shellSurface.y
                })
                menu.open()
            }
        }
    }
}
