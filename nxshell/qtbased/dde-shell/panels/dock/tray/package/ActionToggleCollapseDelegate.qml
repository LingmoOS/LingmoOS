// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as D
import org.deepin.ds 1.0
import org.deepin.ds.dock.tray 1.0 as DDT
import org.deepin.ds.dock 1.0

AppletItemButton {
    id: root
    property bool isHorizontal: false
    property bool collapsed: DDT.TraySortOrderModel.collapsed
    property bool inputEventsEnabled: true
    hoverEnabled: inputEventsEnabled
    autoClosePopup: true

    z: 5

    icon.name: isHorizontal ? "expand-left" : "expand-up"

    padding: itemPadding

    onClicked: {
        DDT.TraySortOrderModel.isCollapsing = true
        DDT.TraySortOrderModel.collapsed = !DDT.TraySortOrderModel.collapsed
        toolTip.close()
    }

    states: [
        State {
            name: "opened"
            when: !collapsed
            PropertyChanges { target: root.contentItem; rotation: 180 }
        },

        State {
            name: "closed"
            when: collapsed
            PropertyChanges { target: root.contentItem; rotation: 0 }
        }
    ]

    transitions: [
        Transition {
            RotationAnimation { duration: 200; }
        }
    ]

    PanelToolTip {
        id: toolTip
        text: qsTr("Collapse tray")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }
    Timer {
        id: toolTipShowTimer
        interval: 200
        onTriggered: {
            var point = root.mapToItem(null, root.width / 2, root.height / 2)
            toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
            toolTip.open()
        }
    }
    HoverHandler {
        id: hoverHandler
        enabled: root.inputEventsEnabled
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
