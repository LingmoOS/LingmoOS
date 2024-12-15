// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.2
import QtQuick.Layouts 1.15

Flickable {
    id: root
    property real spacing: 0
    property bool isGroup: false

    contentHeight: centralItem.height + bottomItem.height + 10
    ScrollBar.vertical: ScrollBar {
        width: 10
    }
    Component {
        id: groupView
        OceanUIGroupView {
            isGroup: root.isGroup
            spacing: root.spacing
        }
    }
    Component {
        id: footer
        OceanUIRowView {}
    }
    Control {
        id: centralItem
        focusPolicy: Qt.TabFocus
        hoverEnabled: false
        focus: true
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 60
            rightMargin: 60
        }
    }
    Rectangle {
        color: this.palette.window
        implicitHeight: bottomItem.implicitHeight + 10
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 60
            rightMargin: 60
        }
        // 防止鼠标穿透
        MouseArea {
            anchors.fill: parent
        }
        Control {
            id: bottomItem
            focusPolicy: Qt.TabFocus
            focus: true
            anchors {
                fill: parent
                topMargin: 5
                bottomMargin: 5
                leftMargin: 10
                rightMargin: 10
            }
        }
        y: (root.contentHeight - root.contentY > root.height ? root.height - this.implicitHeight + root.contentY : root.contentHeight - this.implicitHeight)
    }
    Rectangle {
        id: panel
        property var item: undefined
        property int cnt: 1
        z: 10
        radius: 8
        color: "transparent"
        visible: false
        border.color: this.palette.highlight
        border.width: 2
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 60
            rightMargin: 60
        }
    }
    Timer {
        interval: 100
        repeat: true
        running: panel.item !== undefined
        onTriggered: {
            if (!panel.item || !panel.item.visible || !root.visible || panel.cnt > 5) {
                panel.visible = false
                panel.cnt = 1
                panel.item = undefined
                stop()
            } else {
                let itemY = panel.item.mapToItem(root, 0, 0).y
                let rHeight = root.height - (bottomItem.height + 10)
                if ((itemY + panel.item.height) > rHeight) {
                    root.contentY = itemY + panel.item.height - rHeight + root.contentY
                }
                itemY = panel.item.mapToItem(root, 0, 0).y
                if (itemY < 0) {
                    root.contentY = -itemY
                }

                panel.y = panel.item.mapToItem(root, 0, 0).y + root.contentY
                panel.height = panel.item.height
                panel.visible = panel.cnt & 1
                panel.cnt++
            }
        }
    }
    Connections {
        target: OceanUIApp
        function onActiveItemChanged(item) {
            panel.item = item
        }
    }
    Component.onCompleted: {
        if (oceanuiObj.children.length === 2) {
            if (!oceanuiObj.children[0].page) {
                oceanuiObj.children[0].page = groupView
            }
            centralItem.contentItem = oceanuiObj.children[0].getSectionItem(centralItem)
            if (!oceanuiObj.children[1].page) {
                oceanuiObj.children[1].page = footer
            }
            bottomItem.contentItem = oceanuiObj.children[1].getSectionItem(bottomItem)
        } else {
            console.warn(oceanuiObj.name, " SettingsView must contain two sub items", oceanuiObj.children.length)
        }
    }
}
