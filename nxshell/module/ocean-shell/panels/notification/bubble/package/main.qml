// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0

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

    visible: Applet.visible
    width: 380
    height: Math.max(10, bubbleView.height + bubbleView.anchors.topMargin + bubbleView.anchors.bottomMargin)
    DLayerShellWindow.layer: DLayerShellWindow.LayerTop
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom | DLayerShellWindow.AnchorRight
    DLayerShellWindow.topMargin: windowMargin(0)
    DLayerShellWindow.rightMargin: windowMargin(1)
    DLayerShellWindow.bottomMargin: windowMargin(2)
    palette: DTK.palette
    ColorSelector.family: Palette.CrystalColor
    DWindow.windowEffect: PlatformHandle.EffectNoBorder | PlatformHandle.EffectNoShadow
    color: "transparent"
    DWindow.windowRadius: 0
    DWindow.enabled: false
    // DWindow.enableBlurWindow: true
    screen: Qt.application.screens[0]
    // TODO `Qt.application.screens[0]` maybe invalid, why screen is changed.
    onScreenChanged: {
        root.screen = Qt.binding(function () { return Qt.application.screens[0]})
    }

    ListView  {
        id: bubbleView
        width: 360
        height: contentHeight
        anchors {
            centerIn: parent
            margins: 10
            topMargin: 15
        }

        spacing: 10
        model: Applet.bubbles
        interactive: false
        verticalLayoutDirection: ListView.BottomToTop
        add: Transition {
            id: addTrans
            NumberAnimation { properties: "x"; from: addTrans.ViewTransition.item.width; duration: 600; easing.type: Easing.OutExpo }
        }
        delegate: Bubble {
            width: 360
            bubble: model
        }
    }
}
