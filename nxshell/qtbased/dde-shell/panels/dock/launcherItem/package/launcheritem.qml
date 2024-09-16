// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

AppletItem {
    id: launcher
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 12
    // 1:4 the distance between app : dock height; get width/height≈0.8
    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    Connections {
        target: Panel.rootObject
        function onDockCenterPartPosChanged()
        {
            updateLaunchpadPos()
        }
    }

    Connections {
        target: Panel.rootObject
        function onScreenChanged()
        {
            var launchpad = DS.applet("org.deepin.ds.launchpad")
            if (!launchpad || !launchpad.rootObject)
                return

            launchpad.rootObject.fullscreenFrame.screen = Panel.rootObject.screen
        }
    }

    Connections {
        target: DS.applet("org.deepin.ds.launchpad")
        enabled: target
        function onRootObjectChanged() {
            updateLaunchpadPos()
        }
    }

    property point itemPos: Qt.point(0, 0)
    function updateItemPos()
    {
        var lX = icon.mapToItem(null, icon.width / 2, 0).x
        var lY = icon.mapToItem(null, 0, 0).y
        launcher.itemPos = Qt.point(lX, lY)
    }
    function updateLaunchpadPos()
    {
        updateItemPos()
        var launchpad = DS.applet("org.deepin.ds.launchpad")
        if (!launchpad || !launchpad.rootObject)
            return

        launchpad.rootObject.windowedPos = launcher.itemPos
    }
    Component.onCompleted: {
        updateLaunchpadPos()
    }

    PanelToolTip {
        id: toolTip
        text: qsTr("launchpad")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    D.DciIcon {
        id: icon
        anchors.centerIn: parent
        name: Applet.iconName
        scale: Panel.rootObject.dockItemMaxSize * 9 / 14 / Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE
        // 9:14 (iconSize/dockHeight)
        sourceSize: Qt.size(Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE, Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE)
        onXChanged: updateLaunchpadPos()
        onYChanged: updateLaunchpadPos()
        Timer {
            id: toolTipShowTimer
            interval: 50
            onTriggered: {
                var point = Applet.rootObject.mapToItem(null, Applet.rootObject.width / 2, Applet.rootObject.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            }
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                Applet.toggleLauncher()
                toolTip.close()
            }
        }
        HoverHandler {
            onHoveredChanged: {
                if (hovered) {
                    toolTipShowTimer.start()
                } else {
                    if (toolTipShowTimer.running) {
                        toolTipShowTimer.stop()
                    }

                    toolTip.close()
                }
            }
        }
    }
}
