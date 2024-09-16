// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

Item {
    id: dockCompositor

    property alias dockPosition: pluginManager.dockPosition
    property alias dockColorTheme: pluginManager.dockColorTheme
    property alias dockSize: pluginManager.dockSize

    property ListModel trayPluginSurfaces: ListModel {}
    property ListModel quickPluginSurfaces: ListModel {}
    property ListModel fixedPluginSurfaces: ListModel {}

    property var compositor: waylandCompositor

    signal pluginSurfacesUpdated()
    signal popupCreated(var popup)

    function removeDockPluginSurface(model, object) {
        for (var i = 0; i < model.count; ++i) {
            if (object === model.get(i).shellSurface) {
                model.remove(i)
                break
            }
        }
    }

    function findSurfaceFromModel(model, surfaceId) {
        for (var i = 0; i < model.count; ++i) {
            let item = model.get(i).shellSurface
            if (surfaceId === `${item.pluginId}::${item.itemKey}`) {
                return item
            }
        }
        return null
    }

    function findSurface(surfaceId) {
        let ret = findSurfaceFromModel(trayPluginSurfaces, surfaceId)
        if (ret === null) ret = findSurfaceFromModel(quickPluginSurfaces, surfaceId)
        if (ret === null) ret = findSurfaceFromModel(fixedPluginSurfaces, surfaceId)
        return ret
    }

    function closeShellSurface(shellSurface)
    {
        if (shellSurface) {
            shellSurface.close()
        }
    }

    function updatePopupMinHeight(height)
    {
        pluginManager.setPopupMinHeight(height)
    }

    WaylandCompositor {
        id: waylandCompositor
        socketName: "dockplugin"

        PluginManager {
            id: pluginManager

            onPluginSurfaceCreated: (dockPluginSurface) => {
                console.log("plugin surface created", dockPluginSurface.pluginId, dockPluginSurface.itemKey, dockPluginSurface.pluginType)
                if (dockPluginSurface.pluginType === Dock.Tray) {
                    trayPluginSurfaces.append({shellSurface: dockPluginSurface})
                } else if (dockPluginSurface.pluginType === Dock.Quick) {
                    quickPluginSurfaces.append({shellSurface: dockPluginSurface})
                } else if (dockPluginSurface.pluginType === Dock.Fixed) {
                    fixedPluginSurfaces.append({shellSurface: dockPluginSurface})
                }
                dockCompositor.pluginSurfacesUpdated()
            }

            onPluginSurfaceDestroyed: (dockPluginSurface) => {
                console.log("plugin surface destroyed", dockPluginSurface.pluginId, dockPluginSurface.itemKey, dockPluginSurface.pluginType)
                if (dockPluginSurface.pluginType === Dock.Tray) {
                    removeDockPluginSurface(trayPluginSurfaces, dockPluginSurface)
                } else if (dockPluginSurface.pluginType === Dock.Quick) {
                    removeDockPluginSurface(quickPluginSurfaces, dockPluginSurface)
                } else if (dockPluginSurface.pluginType === Dock.Fixed) {
                    removeDockPluginSurface(fixedPluginSurfaces, dockPluginSurface)
                }
                dockCompositor.pluginSurfacesUpdated()
            }

            onPluginPopupCreated: (popup) => {
                console.log("plugin popup created", popup.pluginId, popup.itemKey, popup.popupType)
                dockCompositor.popupCreated(popup)
            }
        }
    }
}
