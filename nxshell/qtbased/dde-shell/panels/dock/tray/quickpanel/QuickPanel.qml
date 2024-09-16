// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import QtQml

import org.deepin.ds 1.0
import org.deepin.dtk 1.0
import org.deepin.ds.dock 1.0
import org.deepin.ds.dock.tray 1.0 as DDT
import org.deepin.ds.dock.tray.quickpanel 1.0

Item {
    id: root
    implicitWidth: {
        return panelTrayItem.width
    }
    implicitHeight: {
        return panelTrayItem.height
    }
    property alias useColumnLayout: panelTrayItem.useColumnLayout
    property string trayItemPluginId: "sound"
    property int trayItemMargins: 4
    readonly property bool isOpened: panelTrayItem.isOpened

    function isTrayItemPopup(surfaceId)
    {
        if (pluginIdBySurfaceId(surfaceId) !== trayItemPluginId)
            return false

        let pluginId = pluginIdBySurfaceId(surfaceId)
        let itemKey = itemKeyBySurfaceId(surfaceId)
        return !quickpanelModel.isQuickPanelPopup(pluginId, itemKey)
    }
    function pluginIdBySurfaceId(surfaceId)
    {
        let tmp = surfaceId.split("::")
        if (tmp.length !== 2) {
            return ""
        }
        return tmp[0]
    }
    function itemKeyBySurfaceId(surfaceId)
    {
        let tmp = surfaceId.split("::")
        if (tmp.length !== 2) {
            return ""
        }
        return tmp[1]
    }

    function updatePopupMinHeight(value)
    {
        DockCompositor.updatePopupMinHeight(value)
    }

    PanelTrayItem {
        id: panelTrayItem
        shellSurface: quickpanelModel.trayItemSurface
        isOpened: popup.popupVisible
        itemMargins: root.trayItemMargins
        onClicked: function () {
            console.log("show quickpanel")
            popup.DockPanelPositioner.bounding = Qt.binding(function () {
                var point = panelTrayItem.mapToItem(null, panelTrayItem.width / 2, panelTrayItem.height / 2)
                return Qt.rect(point.x, point.y, popup.width, popup.height)
            })
            popup.open()
        }
    }

    PanelPopup {
        id: popup
        width: popupContent.width
        height: popupContent.height
        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        QuickPanelPage {
            id: popupContent
            model: quickpanelModel

            DDT.SurfacePopup {
                objectName: "quickpanel"
                surfaceAcceptor: function (surfaceId) {
                    let pluginId = pluginIdBySurfaceId(surfaceId)
                    let itemKey = itemKeyBySurfaceId(surfaceId)
                    return quickpanelModel.isQuickPanelPopup(pluginId, itemKey)
                }
            }

            onPopupMinHeightChanged: function () {
                root.updatePopupMinHeight(popupContent.popupMinHeight)
            }
        }
    }

    // trayItem's popup
    DDT.TrayItemSurfacePopup {
        id: trayItemSurfacePopup
        surfaceAcceptor: isTrayItemPopup
        surfaceFilter: function (surfaceId) {
            return isOpened
        }
    }
    onIsOpenedChanged: function () {
        if (isOpened) {
            trayItemSurfacePopup.closeTooltip()
        }
    }

    QuickPanelModel {
        id: quickpanelModel
        sourceModel: DockCompositor.quickPluginSurfaces
        trayPluginModel: DockCompositor.trayPluginSurfaces
        trayItemPluginId: root.trayItemPluginId
    }

    Connections {
        target: DockCompositor
        function onPopupCreated(popupSurface)
        {
            if (popupSurface.popupType === Dock.TrayPopupTypeEmbed) {
                console.log("popup created", popupSurface.popupType, popupSurface.pluginId)
                if (!popup.popupVisible) {
                    panelTrayItem.clicked()
                }
                quickpanelModel.requestShowSubPlugin(popupSurface.pluginId, popupSurface)
            }
        }
    }
}
