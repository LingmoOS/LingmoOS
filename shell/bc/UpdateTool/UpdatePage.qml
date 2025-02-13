/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Updator 1.0

// import QtWebView 1.15

Item {
    id: control

    ColumnLayout {
        anchors.fill: parent

        Item {
            Layout.fillHeight: !_listView.visible
        }

        // 插画
        Image {
            Layout.preferredWidth: 143
            Layout.preferredHeight: 172
            source: "qrc:/images/done.svg"
            sourceSize: Qt.size(143, 172)
            Layout.alignment: Qt.AlignHCenter
            asynchronous: true
            visible: !_listView.visible
        }

        Item {
            height: LingmoUI.Units.largeSpacing * 2
            visible: !_listView.visible
        }

        ColumnLayout {
            anchors.fill: parent
            visible: _listView.count !== 0

            Item {
                height: LingmoUI.Units.largeSpacing * 2
                visible: _listView.visible
            }

            Image {
                anchors {
                    // top: parent.top
                    // bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                }
                width: 350
                sourceSize: Qt.size(width, height)
                source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/logo.svg" : "qrc:/images/light/logo.svg"
                smooth: true
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Label {
            text: "<b>" + qsTr("Your system is up to date") + "</b>"
            visible: _listView.count === 0
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("Current Version: %1").arg(updator.version)
            Layout.alignment: Qt.AlignHCenter
            visible: _listView.count === 0
        }

        Label {
            text: qsTr("%1").arg(updator.version) + " | Linux " + updator.kernelVersion
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: 11
            color: LingmoUI.Theme.disabledTextColor
        }

        Label {
            text: "<b>" + qsTr("Package updates are available") + "</b>"
            visible: _listView.count !== 0
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: 14
            font.bold: true
        }

        Item {
            Layout.fillHeight: !_listView.visible
        }

        Item {
            height: LingmoUI.Units.smallSpacing
            visible: _listView.visible
        }
            
        ListView {
            id: _listView
            model: upgradeableModel

            visible: _listView.count !== 0
            spacing: LingmoUI.Units.largeSpacing
            clip: true

            ScrollBar.vertical: ScrollBar {}

            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: Item {
                width: ListView.view.width
                height: 65
                Rectangle {
                    id: _verView
                    anchors.fill: parent
                    // anchors.topMargin: LingmoUI.Units.largeSpacing * 2
                    anchors.leftMargin: LingmoUI.Units.largeSpacing
                    anchors.rightMargin: LingmoUI.Units.largeSpacing
                    anchors.bottom: _logView.top
                    // anchors.bottomMargin: LingmoUI.Units.largeSpacing
                    radius: LingmoUI.Theme.mediumRadius
                    color: LingmoUI.Theme.secondBackgroundColor
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 2
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 2
                    anchors.bottom: _logView.top
                    anchors.bottomMargin: LingmoUI.Units.largeSpacing * 1.5
                    spacing: LingmoUI.Units.smallSpacing

                    Label {
                        // text: model.name
                        text: qsTr("New Version:") + " " + model.version
                        // Layout.fillWidth: true
                        font.pointSize: 11
                        Layout.alignment: Qt.AlignLeft
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    // Size
                    Label {
                        text: model.installedSize
                        color: LingmoUI.Theme.disabledTextColor
                        font.pointSize: 10
                        font.bold: true
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: LingmoUI.Theme.disabledTextColor
                opacity: LingmoUI.Theme.darkMode ? 0.4 : 0.4
                anchors.left: _listView.left
                anchors.right: _listView.right
                anchors.leftMargin: LingmoUI.Units.largeSpacing * 2
                anchors.rightMargin: LingmoUI.Units.largeSpacing * 2
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: _logView.top
                anchors.bottomMargin: LingmoUI.Units.largeSpacing / 10
            }

            Rectangle {
                id: _logView
                anchors.fill: parent
                anchors.topMargin: LingmoUI.Units.largeSpacing * 4
                anchors.leftMargin: LingmoUI.Units.largeSpacing
                anchors.rightMargin: LingmoUI.Units.largeSpacing
                anchors.bottomMargin: LingmoUI.Units.largeSpacing / 2
                radius: LingmoUI.Theme.mediumRadius
                color: LingmoUI.Theme.secondBackgroundColor

                ColumnLayout {
                    anchors.fill: parent
                    // spacing: LingmoUI.Units.smallSpacing

                    Item {
                        Layout.fillHeight: true
                    }

                    ScrollView {
                        id: view
                        anchors.fill: parent        // Define the ScrollView's size
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded
                        // ScrollBar.horizontal.policy: ScrollBar.AsNeeded

                        TextArea {
                            id: textArea
                            text: updator.updateInfo
                            textFormat: TextArea.RichText
                            color: LingmoUI.Theme.disabledTextColor
                            readOnly: true // 设置为只读模式，因为是用于显示日志
                            wrapMode: Text.WordWrap // 确保文本可以换行
                            background: Rectangle {
                                color: "transparent" // 设置背景为透明
                                border.color: "transparent" // 设置边框颜色为透明
                                border.width: 0 // 设置边框宽度为0
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
        

        Item {
            height: LingmoUI.Units.smallSpacing
        }

        Button {
            text: qsTr("Update now")
            Layout.alignment: Qt.AlignHCenter
            visible: _listView.count !== 0
            flat: true
            onClicked: updator.upgrade()
        }

        Item {
            height: LingmoUI.Units.largeSpacing
        }
    }
}
