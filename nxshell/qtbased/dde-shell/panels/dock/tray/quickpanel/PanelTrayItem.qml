// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml

import org.deepin.ds 1.0
import org.deepin.ds.dock 1.0
import org.deepin.ds.dock.tray 1.0
import org.deepin.dtk 1.0

Control {
    id: root
    property bool useColumnLayout: false
    required property var shellSurface
    property int itemMargins
    property bool isOpened
    signal clicked()
    property bool contentHovered
    ColorSelector.hovered: root.contentHovered || root.hovered
    ColorSelector.pressed: mouseHandler.pressed
    property Palette textColor: DockPalette.iconTextPalette
    palette.windowText: ColorSelector.textColor

    PanelToolTip {
        id: toolTip
        text: qsTr("Quick actions")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    contentItem: Grid {
        rows: root.useColumnLayout ? 2 : 1
        spacing: 0
        padding: 0

        Loader {
            id: placeholder
            active: root.shellSurface
            visible: active
            sourceComponent: TrayItemSurface {
                shellSurface: root.shellSurface
                onHoveredChanged: function () {
                    root.contentHovered = hovered
                }
            }
        }
        Control {
            id: quickpanelPlaceholder
            width: placeholder.active ? placeholder.width : 16 + itemMargins
            height: placeholder.active ? placeholder.height : 16 + itemMargins
            contentItem: DciIcon {
                sourceSize: Qt.size(16, 16)
                name: "dock-control-panel"
                palette: DTK.makeIconPalette(root.palette)
                theme: root.ColorSelector.controlTheme
            }
            HoverHandler {
                enabled: !root.isOpened
                onHoveredChanged: function () {
                    root.contentHovered = hovered
                    if (hovered && !root.isOpened) {
                        var point = quickpanelPlaceholder.mapToItem(null, quickpanelPlaceholder.width / 2, quickpanelPlaceholder.height / 2)
                        toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                        toolTip.open()
                    } else {
                        toolTip.close()
                    }
                }
            }
        }
    }
    background: AppletItemBackground {
        isActive: root.isOpened
    }

    component TrayItemSurface: Item {
        implicitWidth: surfaceLayer.width
        implicitHeight: surfaceLayer.height
        property alias shellSurface: surfaceLayer.shellSurface
        property alias hovered: surfaceLayer.hovered

        ShellSurfaceItemProxy {
            id: surfaceLayer
            property var itemGlobalPoint: {
                var a = surfaceLayer
                var x = 0, y = 0
                while(a.parent) {
                    x += a.x
                    y += a.y
                    a = a.parent
                }

                return Qt.point(x, y)
            }

            onItemGlobalPointChanged: {
                if (!shellSurface || !(shellSurface.updatePluginGeometry))
                    return
                shellSurface.margins = root.itemMargins
                shellSurface.updatePluginGeometry(Qt.rect(itemGlobalPoint.x, itemGlobalPoint.y, 0, 0))
            }
        }
    }

    MouseArea {
        id: mouseHandler
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
