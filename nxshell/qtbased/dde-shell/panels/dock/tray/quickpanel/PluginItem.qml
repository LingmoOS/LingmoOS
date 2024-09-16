// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import org.deepin.dtk 1.0
import org.deepin.ds.dock.tray 1.0

Control {
    id: root
    property string pluginId
    property string itemKey
    property alias shellSurface: surfaceLayer.shellSurface
    property alias traySurface: dragLayer.fallbackDragImage
    property bool canDrag: true
    property int radius: 8
    property bool isActive
    function updateSurface()
    {
        surfaceLayer.updateSurfacePosition()
    }

    DragItem {
        id: dragLayer
        anchors.fill: parent
        dragItem: root
        dragTextData: `${root.pluginId}::${root.itemKey}`
        fallbackIconSize: traySurface ? traySurface.size : Qt.size(16, 16)
        enabledDrag: canDrag
    }

    Connections {
        target: shellSurface
        enabled: shellSurface
        function onRecvMouseEvent(type) {
            // type (2: Qt.MouseButtonPress), (3: Qt.MouseButtonRelease)
            if (type === 2) {
                canDrag = false
            } else if (type === 3) {
                canDrag = true
            }
        }
    }

    ShellSurfaceItemProxy {
        id: surfaceLayer
        anchors.centerIn: parent
        anchors.fill: parent
        onWidthChanged: updateSurfaceSize()
        onHeightChanged: updateSurfaceSize()

        function updateSurfaceSize()
        {
            if (!shellSurface || !(shellSurface.updatePluginGeometry))
                return
            shellSurface.updatePluginGeometry(Qt.rect(0, 0, surfaceLayer.width, surfaceLayer.height))
        }
        function updateSurfacePosition()
        {
            if (!shellSurface || !(shellSurface.updatePluginGeometry))
                return

            var pos = surfaceLayer.mapToItem(null, 0, 0)
            shellSurface.updatePluginGeometry(Qt.rect(pos.x, pos.y, surfaceLayer.width, surfaceLayer.height))
        }
    }

    background: Control {
        id: backgroundControl

        // Control's hovered is false when hover ShellSurfaceItem.
        ColorSelector.hovered: surfaceLayer.hovered
        ColorSelector.pressed: surfaceLayer.pressed
        property Palette backgroundColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.3)
            }
            normalDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.05)
            }
            hovered {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.4)
            }
            hoveredDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.1)
            }
            pressed {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.45)
            }
            pressedDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.15)
            }
        }
        property Palette activeBackgroundColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.80)
            }
            normalDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.70)
            }
            hovered {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.90)
            }
            hoveredDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.80)
            }
            pressed {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.95)
            }
            pressedDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.85)
            }
        }
        property Palette insideBorderColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.1)
            }
            normalDark {
                crystal: Qt.rgba(1.0, 1.0, 1.0, 0.05)
            }
        }
        property Palette outsideBorderColor: Palette {
            normal {
                common: ("transparent")
                crystal: Qt.rgba(0.0, 0.0, 0.0, 0.03)
            }
            normalDark {
                crystal: Qt.rgba(0.0, 0.0, 0.0, 0.05)
            }
        }
        Rectangle {
            anchors.fill: parent
            radius: root.radius
            color: isActive ? backgroundControl.ColorSelector.activeBackgroundColor
                            : backgroundControl.ColorSelector.backgroundColor
        }
        InsideBoxBorder {
            anchors.fill: parent
            radius: root.radius
            color: backgroundControl.ColorSelector.insideBorderColor
            borderWidth: 1 / Screen.devicePixelRatio
            z: DTK.AboveOrder
        }
        OutsideBoxBorder {
            anchors.fill: parent
            radius: root.radius
            color: backgroundControl.ColorSelector.outsideBorderColor
            borderWidth: 1 / Screen.devicePixelRatio
            z: DTK.AboveOrder
        }
    }
}
