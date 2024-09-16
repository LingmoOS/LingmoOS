// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.deepin.dtk 1.0
import org.deepin.ds.dock.tray 1.0

Item {
    id: root
    implicitWidth: 330
    implicitHeight: contentHeight

    readonly property int contentHeight: Math.min(Math.max(subPluginMinHeight, subPluginView.height), 600)
    required property var pluginId
    property alias shellSurface: surfaceLayer.shellSurface
    required property var model
    signal requestBack()
    property int subPluginMinHeight
    property int headerMargin: 10

    Component.onCompleted: {
        var surfaceMinHeight = subPluginMinHeight - titleLayer.height - headerMargin
        shellSurface.setEmbedPanelMinHeight(surfaceMinHeight)
    }

    ColumnLayout {
        id: subPluginView
        spacing: 0
        width: root.width

        // header
        RowLayout {
            id: titleLayer
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.topMargin: headerMargin
            Layout.rightMargin: 12
            ActionButton {
                id: actionBtn
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                icon.name: "arrow_ordinary_left"
                icon.width: 12
                icon.height: 12
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                onClicked: {
                    console.log("request back:", pluginId)
                    requestBack()
                }
                background: BoxPanel {
                    radius: actionBtn.width / 2
                    color1: Palette {
                        normal {
                            common: ("transparent")
                        }
                        hovered {
                            crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                        }
                        pressed {
                            crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                        }
                    }
                    color2: color1
                    insideBorderColor: null
                    outsideBorderColor: null
                }
            }

            Item { Layout.fillWidth: true; Layout.preferredHeight: 1 }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: model.getTitle(pluginId)
            }

            Item { Layout.fillWidth: true; Layout.preferredHeight: 1 }
        }

        Item { Layout.fillHeight: true; Layout.preferredWidth: 1 }

        // content
        ShellSurfaceItemProxy {
            id: surfaceLayer
            autoClose: true
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true; Layout.preferredWidth: 1 }
    }
}
