// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

AppletItem {
    id: showdesktop
    readonly property int showDesktopWidth: 10
    property bool useColumnLayout: Panel.position % 2
    property int dockSize: Panel.rootObject.dockItemMaxSize
    property int dockOrder: 30
    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : showDesktopWidth
    implicitHeight: useColumnLayout ? showDesktopWidth : Panel.rootObject.dockSize

    PanelToolTip {
        id: toolTip
        text: qsTr("show desktop")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    AppletItemButton {
        anchors.fill: parent
        radius: 0

        Rectangle {
            property D.Palette lineColor: DockPalette.showDesktopLineColor
            implicitWidth: useColumnLayout ? showdesktop.implicitWidth : 1
            implicitHeight: useColumnLayout ? 1 : showdesktop.implicitHeight

            color: D.ColorSelector.lineColor
        }

        onClicked: {
            Applet.toggleShowDesktop()
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
    }
}
