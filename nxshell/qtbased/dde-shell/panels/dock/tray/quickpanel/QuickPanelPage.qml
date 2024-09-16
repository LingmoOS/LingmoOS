// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtWayland.Compositor

import org.deepin.ds.dock 1.0
import org.deepin.ds 1.0
import org.deepin.dtk 1.0

Item {
    id: root
    width: panelView.width
    height: panelView.height

    required property var model
    property int popupMinHeight: Math.max(360, panelPage.height)

    Connections {
        target: model
        function onRequestShowSubPlugin(pluginId, surface) {
            console.log("show subplugin, plugin", pluginId)
            panelView.push(subPluginPageLoader,
                           {
                               pluginId: pluginId,
                               model: root.model,
                               shellSurface: surface,
                               subPluginMinHeight: root.popupMinHeight
                           },
                           StackView.PushTransition)
        }
    }

    StackView {
        id: panelView
        property int contentHeight: currentItem ? currentItem.height : 10
        width: currentItem ? currentItem.width : 10
        height: panelView.contentHeight
        initialItem: PanelPluginPage {
            id: panelPage
            model: root.model
            StackView.onActivating: function () {
                panelView.contentHeight = Qt.binding(function() { return height })
            }
            StackView.onActivated: function () {
                panelPage.forceLayout()
            }
        }
    }

    Component {
        id: subPluginPageLoader
        SubPluginPage {
            width: panelPage.width
            onRequestBack: function () {
                panelView.pop()
            }
            onVisibleChanged: function () {
                if (!visible)
                    panelView.pop()
            }
            StackView.onActivating: function () {
                panelView.contentHeight = Qt.binding(function() { return contentHeight})
            }
        }
    }
}
