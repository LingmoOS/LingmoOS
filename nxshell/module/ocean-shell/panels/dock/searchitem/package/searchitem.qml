// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0 as D
import org.lingmo.ds.dock 1.0

import Qt.labs.platform 1.1 as LP

AppletItem {
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: Panel.itemAlignment === Dock.CenterAlignment ? 5 : 14
    property bool shouldVisible: Applet.visible
    // 1:4 the distance between app : dock height; get width/height≈0.8
    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    PanelToolTip {
        id: toolTip
        text: qsTr("GrandSearch")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    D.DciIcon {
        id: icon
        anchors.centerIn: parent
        name: "ocean-grand-search"
        scale: Panel.rootObject.dockItemMaxSize * 9 / 14 / Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE
        // 9:14 (iconSize/dockHeight)
        sourceSize: Qt.size(Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE, Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE)
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
                Applet.toggleGrandSearch()
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
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                platformMenuLoader.active = true
                platformMenuLoader.item.open()
            }
        }
    }

    Loader {
        id: platformMenuLoader
        active: false
        sourceComponent: LP.Menu {
            id: platformMenu
            LP.MenuItem {
                text: qsTr("SearchConfig")
                onTriggered: {
                    Applet.toggleGrandSearchConfig()
                }
            }
        }
    }
}
