// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0
import org.lingmo.dtk.style 1.0 as DStyle

import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0 as D
import org.lingmo.ds.dock 1.0

AppletItem {
    id: debuggerApplet
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 12
    // 1:4 the distance between app : dock height; get width/height≈0.8
    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    ListModel {
        id: testmodel
        ListElement {
            display: "Apple"
            launchTimes: 114514
            iconName: "lingmo-music"
            noDisplay: false
        }
    }

    PanelPopup {
        id: launcherDebuggerPanel

        width: 1000
        height: 480
        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        Control {
            anchors.fill: parent
            HorizontalHeaderView {
                z: 999
                id: horizontalHeader
                anchors.left: treeView.left
                anchors.right: treeView.right
                anchors.top: parent.top
                syncView: treeView
                clip: true
            }

            ScrollView {
                anchors.top: horizontalHeader.bottom
                anchors.left: horizontalHeader.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                TreeView {
                    id: treeView
                    model: DListToTableProxyModel {
                        sourceModel: testmodel // Applet.applicationsModel
                        roles: [
                            4096,
                            4097,
                            4098,
                            4099,
                            4100,
                            4101,
                            4102,
                            4103,
                            4104,
                            4105,
                            4106,
                            4107,
                            4108,
                            4109,
                        ]
                        Component.onCompleted: {
                            sourceModel = Applet.applicationsModel
                        }
                    }
                    delegate: TreeViewDelegate {
                        isTreeNode: false
                    }
                }
            }
        }
    }

    D.DciIcon {
        id: icon
        anchors.centerIn: parent
        name: "lingmo-music"
        scale: Panel.rootObject.dockItemMaxSize * 9 / 14 / Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE
        // 9:14 (iconSize/dockHeight)
        sourceSize: Qt.size(Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE, Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE)
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                let rect = Qt.rect(launcherDebuggerPanel.popupX, launcherDebuggerPanel.popupY, launcherDebuggerPanel.width, launcherDebuggerPanel.height)
                launcherDebuggerPanel.DockPanelPositioner.bounding = rect
                launcherDebuggerPanel.open()
            }
        }
    }
}
