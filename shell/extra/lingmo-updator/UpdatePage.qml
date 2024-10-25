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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Updator 1.0

Item {
    id: control

//    Image {
//        anchors.top: parent.top
//        anchors.horizontalCenter: parent.horizontalCenter
//        width: 167
//        height: 26
//        sourceSize: Qt.size(500, 76)
//        source: "qrc:/images/logo.png"
//        asynchronous: true
//        visible: !_listView.visible
//    }

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

        Label {
            text: "<b>" + qsTr("Package updates are available") + "</b>"
            visible: _listView.count !== 0
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "<b>" + qsTr("Your system is up to date") + "</b>"
            visible: _listView.count === 0
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: qsTr("Current Version: %1").arg(updator.version)
            Layout.alignment: Qt.AlignHCenter
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
                height: 50

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing
                    anchors.rightMargin: LingmoUI.Units.largeSpacing
                    color: LingmoUI.Theme.secondBackgroundColor
                    radius: LingmoUI.Theme.mediumRadius
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
                    spacing: LingmoUI.Units.smallSpacing

                    Image {
                        height: 32
                        width: 32
                        sourceSize: Qt.size(width, height)
                        source: "image://icontheme/" + model.name
                        smooth: true
                        antialiasing: true
                    }

                    // Name and version
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            Item {
                                Layout.fillHeight: true
                            }

                            Label {
                                text: model.name
                                Layout.fillWidth: true
                            }

                            Label {
                                text: model.version
                                color: LingmoUI.Theme.disabledTextColor
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }

                    // Size
                    Label {
                        text: model.downloadSize
                        color: LingmoUI.Theme.disabledTextColor
                    }
                }
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
