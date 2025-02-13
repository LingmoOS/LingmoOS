/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     LingmoOS Team <team@lingmo.org>
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
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.Setup 1.0

Item {
    id: control

    Appearance {
        id: appearance
    }

    Background {
        id: background
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        RoundedItem {
            id: ws

            Image {
                id: wallpaper_logo
                width: 350
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: ws.width/20
                sourceSize: Qt.size(width, height)
                source: "qrc:/assets/wallpaper_set.svg"
            }

            Text {
                id: title
                anchors.left: wallpaper_logo.right
                anchors.leftMargin: wallpaper_logo.width/8
                anchors.top: parent.top
                anchors.topMargin: 30
                text: qsTr("Personalize your computer")
                font.pointSize: 20
                font.bold: true
                color: LingmoUI.Theme.textColor
            }

            Text {
                id: txt
                anchors.left: wallpaper_logo.right
                anchors.top: title.bottom
                anchors.topMargin: 5
                anchors.leftMargin: wallpaper_logo.width/8
                text: qsTr("You can choose your desktop wallpaper and switch between light and dark colors.")
                wrapMode: txt.WordWrap
                color: LingmoUI.Theme.textColor
                font.pointSize: 10
            }

            Text {
                id: wpa
                anchors.left: wallpaper_logo.right
                anchors.top: txt.bottom
                anchors.topMargin: 25
                anchors.leftMargin: wallpaper_logo.width/8
                text: qsTr("Wallpaper View")
                wrapMode: txt.WordWrap
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
                font.bold: true
            }

            // DesktopView {
            //     anchors.left: wallpaper_logo.right
            //     anchors.leftMargin: wallpaper_logo.width/8
            //     anchors.top: wpa.bottom
            //     anchors.topMargin: 10
            //     width: 210
            //     // height: 300
            // }
            Rectangle {
                id: deviceItem
                anchors.left: wallpaper_logo.right
                anchors.top: txt.bottom
                anchors.topMargin: 25
                anchors.leftMargin: wallpaper_logo.width/8
                width: win.width/8
                height: win.height/8
                color: LingmoUI.Theme.backgroundColor
                border.width: 4
                border.color: LingmoUI.Theme.textColor
                radius: LingmoUI.Theme.bigRadius

                Image {
                    id: _image
                    width: (win.width/8)-6
                    height: (win.height/8)-6
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: LingmoUI.Theme.darkMode ? "qrc:/assets/wallpaper/MundoDark.jpeg" : "qrc:/assets/wallpaper/MundoLight.jpeg"
                    sourceSize: Qt.size(width, height)
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    mipmap: true
                    cache: true
                    smooth: true
                    opacity: 1.0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutCubic
                        }
                    }

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: _image.width
                            height: _image.height
                            Rectangle {
                                anchors.fill: parent
                                radius: LingmoUI.Theme.bigRadius
                            }
                        }
                    }
                }

                Rectangle {
                    id: dockItem
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: _image.bottom
                    anchors.bottomMargin: 4
                    width: win.width/15 + dockItem.height
                    height: win.height/100
                    color: LingmoUI.Theme.backgroundColor
                    opacity: 0.85
                    border.width: 0.1
                    border.color: LingmoUI.Theme.textColor
                    radius: LingmoUI.Theme.bigRadius/3.5
                }

                Rectangle {
                    id: icon1
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: dockItem.left
                    anchors.leftMargin: 2
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#D4D4D4"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon2
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon1.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#DBC81E"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon3
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon2.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#464444"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon4
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon3.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#4891FF"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon5
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon4.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#FD5D93"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon6
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon5.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#0066FF"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon7
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon6.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#A537FF"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon8
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon7.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#FF4995"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon9
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon8.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#862D04"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon10
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon9.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#23A829"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon11
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon10.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#225366"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon12
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon11.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#DD9426"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon13
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon12.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#E74343"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                Rectangle {
                    id: icon14
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon13.right
                    anchors.leftMargin: 3
                    width: dockItem.height-4
                    height: icon1.width
                    color: "#24B4A1"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 2
                }

                // FastBlur {
                //     anchors.fill: dockItem
                //     source: dockItem
                //     width: source.width
                //     height: source.height
                //     radius: 64
                // }
            }

            Text {
                id: defd
                anchors.top: deviceItem.bottom
                anchors.topMargin: 5
                anchors.horizontalCenter: deviceItem.horizontalCenter
                text: qsTr("Device")
                wrapMode: txt.WordWrap
                color: LingmoUI.Theme.textColor
                font.pointSize: 10
                font.bold: true
            }

            Text {
                id: themes
                anchors.left: wallpaper_logo.right
                anchors.top: wpa.bottom
                anchors.topMargin: 160
                anchors.leftMargin: wallpaper_logo.width/8
                text: qsTr("Theme")
                wrapMode: txt.WordWrap
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
                font.bold: true
            }

            RowLayout {
                anchors.left: wallpaper_logo.right
                anchors.leftMargin: wallpaper_logo.width/8
                anchors.top: themes.bottom
                anchors.topMargin: 10

                IconCheckBox {
                    source: "qrc:/assets/light_mode.svg"
                    text: qsTr("Light")
                    checked: !LingmoUI.Theme.darkMode
                    onClicked: appearance.switchDarkMode(false)
                }

                IconCheckBox {
                    source: "qrc:/assets/dark_mode.svg"
                    text: qsTr("Dark")
                    checked: LingmoUI.Theme.darkMode
                    onClicked: appearance.switchDarkMode(true)
                }
            }

            Button {
                // anchors.centerIn: parent
                id: exBt
                anchors.right: parent.right
                anchors.rightMargin: parent.width/20
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 35
                flat: true
                text: qsTr("Next")
                onClicked: {
                    stackView.push(donePage)
                }
            }

            Button {
                // anchors.centerIn: parent
                id: back
                anchors.right: exBt.left
                anchors.rightMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 35
                flat: false
                text: qsTr("Back")
                onClicked: {
                    stackView.pop()
                }
            }
            
            // Button {
            //     flat: false
            //     text: qsTr("Exit")
            //     onClicked: Qt.quit()
            // }
        }
    }
}