// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DStyle
import org.deepin.dtk.private 1.0 as DP
import org.deepin.ds.dock 1.0

import Qt.labs.platform 1.1 as LP

AppletItem {
    id: searchItem
    property int dockSize: Panel.rootObject.dockSize
    property int dockOrder: 3
    implicitWidth: Panel.rootObject.useColumnLayout ? dockSize : 30 
    implicitHeight: Panel.rootObject.useColumnLayout ? 30 : dockSize
    property bool shouldVisible: Applet.visible && Panel.itemAlignment === Dock.CenterAlignment

    property D.Palette toolButtonColor: DockPalette.toolButtonColor
    property D.Palette toolButtonBorderColor: DockPalette.toolButtonBorderColor


    property D.Palette backgroundColor: D.Palette {
        normal {
            common: Qt.rgba(1, 1, 1, 0.4)
        }
        normalDark{
            common: Qt.rgba(1, 1, 1, 0.2)
        }
    }

    PanelToolTip {
        id: toolTip
        text: qsTr("GrandSearch")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    AppletItemButton {
        id: button
        anchors.centerIn: parent
        icon.name: "search"
        isActive: Applet.grandSearchVisible

        onClicked: {
            toolTip.close()
            Applet.toggleGrandSearch()
        }
        onHoveredChanged: {
            if (hovered) {
                var point = Applet.rootObject.mapToItem(null, Applet.rootObject.width / 2, Applet.rootObject.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            } else {
                toolTip.close()
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
