// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0 as D

Window {
    id: root
    visible: Applet.visible
    D.DWindow.enabled: true
    D.DWindow.windowRadius: 18
    D.DWindow.enableBlurWindow: true
    color: "transparent"
    DLayerShellWindow.bottomMargin: 140
    DLayerShellWindow.layer: DLayerShellWindow.LayerOverlay
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom
    screen: Qt.application.screens[0]
    // TODO `Qt.application.screens[0]` maybe invalid, why screen is changed.
    onScreenChanged: {
        root.screen = Qt.binding(function () { return Qt.application.screens[0]})
    }

    width: osdView ? osdView.width : 100
    height: osdView ? osdView.height : 100

    property Item osdView

    Repeater {
        model: Applet.appletItems
        delegate: Loader {
            active: modelData.update(Applet.osdType)
            onActiveChanged: {
                if (active) {
                    root.osdView = this
                }
            }

            sourceComponent: Control {
                contentItem: model.data
                padding: 10
                background: D.FloatingPanel {
                    implicitWidth:  100
                    implicitHeight: 40
                }
            }
        }
    }
}
