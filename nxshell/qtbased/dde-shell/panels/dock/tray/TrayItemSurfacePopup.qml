// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor

import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0
import org.deepin.ds.dock.tray 1.0

Item {
    id: root

    property var surfaceAcceptor: function (surfaceId) {
        return true
    }
    property var surfaceFilter: function (surfaceId) {
        return false
    }
    readonly property bool isOpened: popup.popupVisible
    function closeTooltip() {
        if (toolTip.toolTipVisible) {
            toolTip.close()
        }
    }

    PanelPopup {
        id: popup
        property alias shellSurface: popupContent.shellSurface
        width: popupContent.width
        height: popupContent.height
        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        Item {
            anchors.fill: parent
            ShellSurfaceItemProxy {
                id: popupContent
                anchors.centerIn: parent
                autoClose: true
                onSurfaceDestroyed: function () {
                    popup.close()
                }
            }
        }
    }

    PanelMenu {
        id: popupMenu
        property alias shellSurface: popupMenuContent.shellSurface
        width: popupMenuContent.width
        height: popupMenuContent.height
        menuX: DockPositioner.x
        menuY: DockPositioner.y

        Item {
            anchors.fill: parent
            ShellSurfaceItemProxy {
                id: popupMenuContent
                anchors.centerIn: parent
                autoClose: true
                onSurfaceDestroyed: function () {
                    popupMenu.close()
                }
            }
            Loader {
                active: popupMenu.menuVisible
                sourceComponent: SurfaceSubPopup {
                    objectName: "tray's subPopup"
                    transientParent: popupMenu.menuWindow
                    surfaceAcceptor: function (surfaceId) {
                        if (root.surfaceAcceptor && !root.surfaceAcceptor(surfaceId))
                            return false
                        return true
                    }
                }
            }

            Connections {
                target: popupMenu.menuWindow
                enabled: popupMenu.readyBinding
                function onUpdateGeometryFinished() {
                    if (!popupMenu.shellSurface)
                        return

                    popupMenu.shellSurface.updatePluginGeometry(Qt.rect(popupMenu.menuWindow.x, popupMenu.menuWindow.y, 0, 0))
                }
            }
        }
    }

    PanelToolTip {
        id: toolTip
        property alias shellSurface: toolTipContent.shellSurface
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y

        ShellSurfaceItemProxy {
            id: toolTipContent
            anchors.centerIn: parent
            autoClose: true
            onSurfaceDestroyed: function () {
                toolTip.close()
            }
        }
    }

    Connections {
        target: DockCompositor
        function onPopupCreated(popupSurface) {
            let surfaceId = `${popupSurface.pluginId}::${popupSurface.itemKey}`
            if (surfaceAcceptor && !surfaceAcceptor(surfaceId))
                return

            if (surfaceFilter && surfaceFilter(surfaceId))
                return

            if (popupSurface.popupType === Dock.TrayPopupTypeTooltip) {
                console.log("tray's tooltip created", popupSurface.pluginId, popupSurface.itemKey)
                toolTip.shellSurface = popupSurface
                toolTip.DockPanelPositioner.bounding = Qt.binding(function () {
                    var point = Qt.point(toolTip.shellSurface.x, toolTip.shellSurface.y)
                    return Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                })
                toolTip.open()
            } else if (popupSurface.popupType === Dock.TrayPopupTypePanel) {
                console.log("tray's popup created", popupSurface.pluginId, popupSurface.itemKey)
                popup.shellSurface = popupSurface
                popup.DockPanelPositioner.bounding = Qt.binding(function () {
                    var point = Qt.point(popup.shellSurface.x, popup.shellSurface.y)
                    return Qt.rect(point.x, point.y, popup.width, popup.height)
                })
                popup.open()
            } else if (popupSurface.popupType === Dock.TrayPopupTypeMenu) {
                console.log("tray's menu created", popupSurface.pluginId, popupSurface.itemKey)
                popupMenu.shellSurface = popupSurface
                popupMenu.DockPositioner.bounding = Qt.binding(function () {
                    var point = Qt.point(popupMenu.shellSurface.x, popupMenu.shellSurface.y)
                    return Qt.rect(point.x, point.y, popupMenu.width, popupMenu.height)
                })
                popupMenu.open()
            }
        }
    }
}
