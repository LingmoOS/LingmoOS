// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

AppletItem {
    id: workspaceitem
    property int dockSize: Panel.rootObject.dockSize
    property int dockOrder: 2
    property int frameSize: 20
    property int itemSize: 16
    property int space: 4
    // todo: visible property to be set
    property bool shouldVisible: listView.count > 1 && Panel.itemAlignment === Dock.CenterAlignment

    // visible:listView.count > 1
    implicitWidth: Panel.position === Dock.Top || Panel.position === Dock.Bottom ? listView.count * frameSize + space * (listView.count - 1) : dockSize
    implicitHeight: Panel.position === Dock.Left || Panel.position === Dock.Right ? listView.count * frameSize + space * (listView.count - 1) : dockSize

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onHoveredChanged: {
            Applet.dataModel.preview(containsMouse)
        }
    }

    component WorkspaceDelegate: Item {
        id: content
        required property int index
        required property string workspaceName
        required property string screenImage
        property bool isCurrent: content.ListView.view.currentIndex === content.index

        implicitWidth: Panel.position === Dock.Top || Panel.position === Dock.Bottom ? frameSize : dockSize
        implicitHeight: Panel.position === Dock.Left || Panel.position === Dock.Right ? frameSize : dockSize

        Control {
            anchors.centerIn: parent
            implicitWidth: frameSize
            implicitHeight: frameSize

            Rectangle {
                id: workspaceRectangle
                property D.Palette backgroundColor: DockPalette.workspaceRectangleColor
                property D.Palette unSelectedBorderColor: DockPalette.workspaceUnselectedBorderColor
                property D.Palette selectedBorderColor: DockPalette.workspaceSelectedBorderColor

                anchors.centerIn: parent
                border.width: 1
                border.color: isCurrent ? D.ColorSelector.selectedBorderColor : D.ColorSelector.unSelectedBorderColor
                implicitWidth: frameSize
                implicitHeight: isCurrent ? itemSize + 4 : itemSize
                color: isCurrent ? "transparent" : D.ColorSelector.backgroundColor
                radius: 3
            }

            Image {
                anchors.fill: parent
                id: workspaceImage
                source: screenImage
                visible: false
                fillMode: Image.PreserveAspectCrop
            }

            OpacityMask {
                anchors.fill: workspaceImage
                z: -1
                source: workspaceImage
                maskSource: Rectangle {
                    implicitWidth: workspaceRectangle.implicitWidth
                    implicitHeight: workspaceRectangle.implicitHeight
                    radius: workspaceRectangle.radius
                }
                visible: isCurrent
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    Applet.dataModel.currentIndex =  content.index
                }
                onEntered: {
                    toolTip.text = content.workspaceName
                    var point = content.mapToItem(null, content.width / 2, content.height / 2)
                    toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                    toolTip.open()
                }

                onExited: {
                    toolTip.close()
                }
            }
        }
    }

    PanelToolTip {
        id: toolTip
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    Connections {
        target: Applet.dataModel
        function onCurrentIndexChanged() {
            toolTip.close()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        ListView {
            id: listView
            anchors.fill: parent
            spacing: 4
            orientation: Panel.position === Dock.Top || Panel.position === Dock.Bottom ? ListView.Horizontal : ListView.Vertical
            delegate: WorkspaceDelegate{}

            model: Applet.dataModel
            currentIndex: Applet.dataModel.currentIndex
            interactive: false
        }
    }
}
