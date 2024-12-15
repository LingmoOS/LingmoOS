// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds 1.0
import org.lingmo.ds.notificationcenter

Window {
    id: root

    function windowMargin(position) {
        let dockApplet = DS.applet("org.lingmo.ds.dock")
        if (!dockApplet)
            return 0

        let dockScreen = dockApplet.screenName
        let screen = root.screen.name
        let dockHideState = dockApplet.hideState
        let dockIsHide = dockHideState === 2
        if (dockScreen !== screen || dockIsHide)
            return 0

        let dockSize = dockApplet.dockSize
        let dockPosition = dockApplet.position
        return dockPosition === position ? dockSize : 0
    }

    // visible: true
    visible: Panel.visible
    flags: Qt.Tool

    width: 360 + view.anchors.leftMargin + view.anchors.rightMargin
    // height: 800
    DLayerShellWindow.layer: DLayerShellWindow.LayerOverlay
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorRight | DLayerShellWindow.AnchorTop | DLayerShellWindow.AnchorBottom
    DLayerShellWindow.topMargin: windowMargin(0) + 10
    DLayerShellWindow.rightMargin: windowMargin(1) + 10
    DLayerShellWindow.bottomMargin: windowMargin(2) + 10
    palette: DTK.palette
    ColorSelector.family: Palette.CrystalColor
    DWindow.windowEffect: PlatformHandle.EffectNoBorder | PlatformHandle.EffectNoShadow
    // DWindow.windowRadius: DTK.platformTheme.windowRadius
    // DWindow.enableSystemResize: false
    // DWindow.enableSystemMove: false
    DWindow.windowRadius: 0
    DWindow.enabled: false
    color: "transparent"
    // DWindow.enableBlurWindow: true
    screen: Qt.application.screens[0]
    // TODO `Qt.application.screens[0]` maybe invalid, why screen is changed.
    onScreenChanged: {
        root.screen = Qt.binding(function () { return Qt.application.screens[0]})
    }

    onVisibleChanged: function (v) {
        if (v) {
            requestActivate()
        }
    }

    onActiveChanged: function () {
        if (!root.active) {
            Panel.close()
        }
    }

    // close Panel when click dock.
    Connections {
        target: DS.applet("org.lingmo.ds.dock")
        function onRequestClosePopup() {
            Panel.close()
        }
    }

    Item {
        id: view
        width: parent.width
        anchors {
            top: parent.top
            left: parent.left
            margins: 10
            bottom: parent.bottom
        }

        NotifyStaging {
            id: notifyStaging
            anchors {
                top: parent.top
                left: parent.left
            }
            implicitWidth: 360
            Connections {
                target: Panel
                function onVisibleChanged() {
                    if (Panel.visible) {
                        notifyStaging.model.open()
                    } else {
                        notifyStaging.model.close()
                    }
                }
            }
        }

        NotifyCenter {
            id: notifyCenter
            anchors {
                top: notifyStaging.bottom
                left: parent.left
                bottom: parent.bottom
            }

            Connections {
                target: Panel
                function onVisibleChanged() {
                    if (Panel.visible) {
                        notifyCenter.model.open()
                    } else {
                        notifyCenter.model.close()
                    }
                }
            }

            implicitWidth: 360
            maxViewHeight: root.height
        }
    }
}
