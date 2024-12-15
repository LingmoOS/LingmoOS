// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.ds 1.0
import org.lingmo.ds.dock 1.0
import org.lingmo.dtk 1.0 as D

ContainmentItem {
    id: taskmanager
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 16

    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : appContainer.implicitWidth
    implicitHeight: useColumnLayout ? appContainer.implicitHeight : Panel.rootObject.dockSize

    OverflowContainer {
        id: appContainer
        anchors.centerIn: parent
        useColumnLayout: taskmanager.useColumnLayout
        spacing: Panel.rootObject.itemSpacing
        add: Transition {
            NumberAnimation {
                properties: "scale,opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        remove: Transition {
            NumberAnimation {
                properties: "scale,opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                easing.type: Easing.OutQuad
            }
        }
        move: displaced
        model: DelegateModel {
            id: visualModel
            model: taskmanager.Applet.dataModel
            delegate: DropArea {
                id: delegateRoot
                required property bool active
                required property bool attention
                required property string itemId
                required property string name
                required property string iconName
                required property string menus
                required property list<string> windows
                keys: ["text/x-ocean-dock-dnd-appid"]
                z: attention ? -1 : 0
                property bool visibility: itemId !== taskmanager.Applet.desktopIdToAppId(launcherDndDropArea.launcherDndDesktopId)

                states: [
                    State {
                        name: "item-visible"
                        when: delegateRoot.visibility
                        PropertyChanges { target: delegateRoot; opacity: 1.0; scale: 1.0; }
                    },
                    State {
                        name: "item-invisible"
                        when: !delegateRoot.visibility
                        PropertyChanges { target: delegateRoot; opacity: 0.0; scale: 0.0; }
                    }
                ]

                Behavior on opacity { NumberAnimation { duration: 200 } }
                Behavior on scale { NumberAnimation { duration: 200 } }

                // TODO: 临时溢出逻辑，待后面修改
                // 1:4 the distance between app : dock height; get width/height≈0.8
                implicitWidth: useColumnLayout ? Panel.rootObject.dockItemMaxSize : Panel.rootObject.dockItemMaxSize * 0.8
                implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockItemMaxSize

                onEntered: function(drag) {
                    visualModel.items.move((drag.source as AppItem).visualIndex, app.visualIndex)
                }

                property int visualIndex: DelegateModel.itemsIndex

                AppItem {
                    id: app
                    displayMode: Panel.indicatorStyle
                    colorTheme: Panel.colorTheme
                    active: delegateRoot.active
                    attention: delegateRoot.attention
                    itemId: delegateRoot.itemId
                    name: delegateRoot.name
                    iconName: delegateRoot.iconName
                    menus: delegateRoot.menus
                    windows: delegateRoot.windows
                    visualIndex: delegateRoot.visualIndex
                    ListView.delayRemove: Drag.active
                    Component.onCompleted: {
                        clickItem.connect(taskmanager.Applet.clickItem)
                    }
                    onDragFinished: function() {
                        launcherDndDropArea.resetDndState()
                    }
                    anchors.fill: parent // This is mandatory for draggable item center in drop area
                }
            }
        }

        DropArea {
            id: launcherDndDropArea
            anchors.fill: parent
            keys: ["text/x-ocean-dock-dnd-appid"]
            property string launcherDndDesktopId: ""
            property string launcherDndDragSource: ""

            function resetDndState() {
                launcherDndDesktopId = ""
                launcherDndDragSource = ""
            }

            onEntered: function(drag) {
                let desktopId = drag.getDataAsString("text/x-ocean-dock-dnd-appid")
                launcherDndDragSource = drag.getDataAsString("text/x-ocean-dock-dnd-source")
                launcherDndDesktopId = desktopId
                if (taskmanager.Applet.requestDockByDesktopId(desktopId) === false) {
                    resetDndState()
                }
            }

            onPositionChanged: function(drag) {
                if (launcherDndDesktopId === "") return
                let curX = taskmanager.useColumnLayout ? drag.y : drag.x
                curX *= Screen.devicePixelRatio
                let cellWidth = Panel.rootObject.dockItemMaxSize
                let curCell = curX / cellWidth
                let appId = taskmanager.Applet.desktopIdToAppId(launcherDndDesktopId)
                taskmanager.Applet.dataModel.moveTo(appId, curCell)
            }

            onDropped: function(drop) {
                if (launcherDndDesktopId === "") return
                let curX = taskmanager.useColumnLayout ? drop.y : drop.x
                curX *= Screen.devicePixelRatio
                let cellWidth = Panel.rootObject.dockItemMaxSize
                let curCell = curX / cellWidth
                let appId = taskmanager.Applet.desktopIdToAppId(launcherDndDesktopId)
                taskmanager.Applet.dataModel.moveTo(appId, curCell)
                resetDndState()
            }

            onExited: function() {
                if (launcherDndDesktopId !== "" && launcherDndDragSource !== "taskbar") {
                    taskmanager.Applet.requestUndockByDesktopId(launcherDndDesktopId)
                }
                resetDndState()
            }
        }
    }

    Component.onCompleted: {
        Panel.rootObject.dockItemMaxSize = Qt.binding(function(){
            return Math.min(Panel.rootObject.dockSize, Panel.rootObject.dockLeftSpaceForCenter * 1.2 / (Panel.rootObject.dockCenterPartCount - 1 + taskmanager.Applet.dataModel.rowCount()) - 2)
        })
    }
}
