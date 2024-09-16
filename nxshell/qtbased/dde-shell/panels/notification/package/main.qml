// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

Window {
    id: root
    visible: Applet.visible
    width: 340 + 48
    height: Math.max(10, bubbleView.height + 48)
    DLayerShellWindow.bottomMargin: 10
    DLayerShellWindow.layer: DLayerShellWindow.LayerOverlay
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom | DLayerShellWindow.AnchorRight

    ListView  {
        anchors.centerIn: parent
        id: bubbleView
        width: root.width - 48
        height: contentHeight
        spacing: 10
        model: Applet.bubbles
        interactive: false
        verticalLayoutDirection: ListView.BottomToTop
        add: Transition {
            NumberAnimation { properties: "x"; from: 100; duration: 500 }
        }
        delegate: Bubble {
            width: 340
            bubble: model
        }
    }
    color: "transparent"
}
