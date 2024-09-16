// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor
import Qt.labs.platform 1.1 as LP
import org.deepin.dtk 1.0 as D

import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0
import org.deepin.ds.dock.tray 1.0
import org.deepin.ds.dock.tray 1.0 as DDT

AppletItem {
    id: tray

    readonly property int nextAppletSpacing: 6
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 25
    readonly property string quickpanelTrayItemPluginId: "sound"
    readonly property var filterTrayPlugins: [quickpanelTrayItemPluginId]

    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : trayContainter.implicitWidth + nextAppletSpacing
    implicitHeight: useColumnLayout ? trayContainter.implicitHeight + nextAppletSpacing: Panel.rootObject.dockSize
    Component.onCompleted: {
        Applet.trayPluginModel = Qt.binding(function () {
            return DockCompositor.trayPluginSurfaces
        })
        Applet.quickPluginModel = Qt.binding(function () {
            return DockCompositor.quickPluginSurfaces
        })
        Applet.fixedPluginModel = Qt.binding(function () {
            return DockCompositor.fixedPluginSurfaces
        })
    }

    PanelPopup {
        id: stashedPopup
        width: stashedContainer.width
        height: stashedContainer.height

        property alias dropHover: stashContainer.dropHover
        property alias stashItemDragging: stashContainer.stashItemDragging

        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        property point collapsedBtnCenterPoint: Qt.point(0, 0)

        Control {
            id: stashedContainer
            padding: 10
            contentItem: StashContainer {
                id: stashContainer
                color: "transparent"
                model: DDT.SortFilterProxyModel {
                    sourceModel: DDT.TraySortOrderModel
                    filterRowCallback: (sourceRow, sourceParent) => {
                        let index = sourceModel.index(sourceRow, 0, sourceParent)
                        return sourceModel.data(index, DDT.TraySortOrderModel.SectionTypeRole) === "stashed" &&
                               sourceModel.data(index, DDT.TraySortOrderModel.VisibilityRole) === true
                    }
                }
                anchors.centerIn: parent
                onRowCountChanged: {
                    if (stashContainer.rowCount === 0 || stashContainer.columnCount === 0) {
                        stashedPopup.close()
                    }
                }
            }
        }

        Component.onCompleted: {
            DockPanelPositioner.bounding = Qt.binding(function () {
                return Qt.rect(collapsedBtnCenterPoint.x, collapsedBtnCenterPoint.y, stashedPopup.width, stashedPopup.height)
            })
        }
    }
    Connections {
        target: DDT.TraySortOrderModel
        function onActionsAlwaysVisibleChanged(val) {
            if (!val) {
                closeStashPopupTimer.start()
            }
        }
    }

    // Bug to prevent icons from returning to the application tray when the tray is already hidden, which can cause layout confusion
    Timer {
        id: closeStashPopupTimer
        interval: 10
        repeat: false
        onTriggered: {
            stashedPopup.close()
        }
    }

    TrayContainer {
        id: trayContainter
        isHorizontal: !tray.useColumnLayout
        model: DDT.TraySortOrderModel
        collapsed: DDT.TraySortOrderModel.collapsed
        trayHeight: isHorizontal ? tray.implicitHeight : tray.implicitWidth
        surfaceAcceptor: isTrayPluginPopup
        color: "transparent"
        Component.onCompleted: {
            DDT.TrayItemPositionManager.layoutHealthCheck(1500)
        }
    }

    function isTrayPluginPopup(surfaceId) {
        if (stashContainer.isStashPopup(surfaceId))
            return false
        if (DockCompositor.findSurfaceFromModel(DockCompositor.trayPluginSurfaces, surfaceId))
            return true
        if (DockCompositor.findSurfaceFromModel(DockCompositor.fixedPluginSurfaces, surfaceId))
            return true
        return false
    }

    Connections {
        target: DockCompositor
        function onPluginSurfacesUpdated() {
            let surfacesData = []
            for (let i = 0; i < DockCompositor.trayPluginSurfaces.count; i++) {
                let item = DockCompositor.trayPluginSurfaces.get(i).shellSurface
                if (filterTrayPlugins.indexOf(item.pluginId) >= 0)
                    continue;
                let surfaceId = `${item.pluginId}::${item.itemKey}`
                let forbiddenSections = ["fixed"]
                let preferredSection = item.pluginId === "application-tray" ? "stashed" : "collapsable"

                if (item.pluginSizePolicy === Dock.Custom) {
                    forbiddenSections = ["stashed", "fixed"]
                    preferredSection = "pinned"
                }

                if (item.pluginFlags & 0x1000) { // force dock.
                    forbiddenSections = ["stashed", "collapsable", "fixed"]
                    preferredSection = "pinned"
                }

                surfacesData.push({"surfaceId": surfaceId, "delegateType": "legacy-tray-plugin", "sectionType": preferredSection, "forbiddenSections": forbiddenSections, "isForceDock": item.pluginFlags & 0x1000})
            }
            // actually only for datetime plugin currently
            for (let i = 0; i < DockCompositor.fixedPluginSurfaces.count; i++) {
                let item = DockCompositor.fixedPluginSurfaces.get(i).shellSurface
                let surfaceId = `${item.pluginId}::${item.itemKey}`
                let forbiddenSections = ["stashed", "collapsable", "pinned"]
                let preferredSection = "fixed"

                surfacesData.push({"surfaceId": surfaceId, "delegateType": "legacy-tray-plugin", "sectionType": preferredSection, "forbiddenSections": forbiddenSections, "isForceDock": item.pluginFlags & 0x1000})
            }
            DDT.TraySortOrderModel.availableSurfaces = surfacesData
            console.log("onPluginSurfacesUpdated", surfacesData.length)
        }
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: Panel.rootObject
        sizeFollowsWindow: true
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: Panel.popupWindow
        sizeFollowsWindow: false
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: Panel.toolTipWindow
        sizeFollowsWindow: false
    }

    WaylandOutput {
        compositor: DockCompositor.compositor
        window: Panel.menuWindow
        sizeFollowsWindow: false
    }
}
