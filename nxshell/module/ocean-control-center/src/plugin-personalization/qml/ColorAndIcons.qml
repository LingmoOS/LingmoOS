// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUITitleObject {
        name: "accentColorTitle"
        weight: 10
        parentName: "personalization/colorAndIcons"
        displayName: qsTr("Accent Color")
    }

    OceanUIObject {
        name: "accentColor"
        parentName: "personalization/colorAndIcons"
        weight: 100
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal
        page: ListView {
            id: listview
            readonly property var colors: ["#DF4187", "#EA691F", "#F3B517", "#49B125", "#00A48A", "#1F6EE7", "#402FDB", "#7724B1", "#757575", "CUSTOM"]
            readonly property var darkColors: ["#A82B62", "#CC4D03", "#D09C00", "#459F29", "#188876", "#024CCA", "#443BBA", "#6A2487", "#868686", "CUSTOM"]
            property var cutColors: oceanuiData.currentAppearance === ".dark" ? darkColors : colors
            implicitHeight: 60
            leftMargin: 10
            clip: true
            model: cutColors.length
            orientation: ListView.Horizontal
            layoutDirection: Qt.LeftToRight
            spacing: 12

            delegate: Item {
                anchors.verticalCenter: parent.verticalCenter
                property string activeColor: oceanuiData.model.activeColor
                property string currentColor: listview.cutColors[index]
                width: 30
                height: 30
                Rectangle {
                    anchors.fill: parent
                    border.width: 2
                    visible: activeColor === currentColor || (currentColor == "CUSTOM" && listview.cutColors.indexOf(activeColor) === -1)
                    border.color: currentColor == "CUSTOM" ? activeColor : currentColor
                    radius: width / 2
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 4
                    radius: width / 2
                    color: listview.cutColors[index] === "CUSTOM" ? "transparent" : listview.cutColors[index]
                    anchors.centerIn: parent

                    D.BoxShadow {
                        anchors.fill: parent
                        shadowColor: parent.color
                        shadowOffsetY: 2
                        shadowBlur: 6
                        cornerRadius: parent.radius
                    }

                    Canvas {
                        anchors.fill: parent
                        visible: listview.cutColors[index] === "CUSTOM"
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);

                            var centerX = width / 2;
                            var centerY = height / 2;
                            var radius = Math.min(width, height) / 2;

                            var gradient = ctx.createConicalGradient(centerX, centerY, 0, centerX, centerY, radius);

                            gradient.addColorStop(0.0, "red");
                            gradient.addColorStop(0.167, "yellow");
                            gradient.addColorStop(0.333, "green");
                            gradient.addColorStop(0.5, "cyan");
                            gradient.addColorStop(0.667, "blue");
                            gradient.addColorStop(0.833, "magenta");
                            gradient.addColorStop(1.0, "red");

                            ctx.fillStyle = gradient;
                            ctx.beginPath();
                            ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
                            ctx.closePath();
                            ctx.fill();
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (listview.cutColors[index] === "CUSTOM") {
                                colorDialog.color = oceanuiData.model.activeColor
                                colorDialog.open()
                            } else {
                                oceanuiData.worker.setActiveColor(listview.cutColors[index])
                                oceanuiData.worker.setActiveColors(listview.colors[index] + "," + listview.darkColors[index])
                            }
                        }
                    }
                }
            }
            OceanUIColorDialog {
                id: colorDialog
                anchors.centerIn: Overlay.overlay
                width: 300
                height: 300
                onAccepted: {
                    console.warn(colorDialog.color)
                    oceanuiData.worker.setActiveColor(colorDialog.color)
                    oceanuiData.worker.setActiveColors(colorDialog.color + "," + colorDialog.color)
                }
            }
        }
    }

    OceanUITitleObject {
        name: "iconSettingsTitle"
        parentName: "personalization/colorAndIcons"
        displayName: qsTr("Icon Settings")
        weight: 200
    }

    OceanUIObject {
        name: "iconTheme"
        parentName: "personalization/colorAndIcons"
        displayName: qsTr("Icon Theme")
        description: qsTr("Customize your theme icon")
        icon: "theme_icon"
        weight: 300
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.MenuEditor
        page: Label {
            text: oceanuiData.model.iconModel.currentTheme
        }

        OceanUIObject {
            name: "iconThemeSelect"
            parentName: "personalization/colorAndIcons/iconTheme"
            weight: 1
            OceanUIObject {
                name: "cursorThemeSelect"
                parentName: "personalization/colorAndIcons/iconTheme/iconThemeSelect"
                weight: 1
                pageType: OceanUIObject.Item
                page: IconThemeGridView {
                    model: oceanuiData.iconThemeViewModel
                    onRequsetSetTheme: (id)=> {
                                           oceanuiData.worker.setIconTheme(id)
                                       }
                }
            }
        }
    }
    OceanUIObject {
        name: "cursorTheme"
        parentName: "personalization/colorAndIcons"
        displayName: qsTr("Cursor Theme")
        description: qsTr("Customize your theme cursor")
        icon: "topic_cursor"
        weight: 400
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.MenuEditor
        visible: oceanuiData.platformName() !== "wayland"
        page: Label {
            text: oceanuiData.model.cursorModel.currentTheme
        }

        OceanUIObject {
            name: "cursorThemeSelect"
            parentName: "personalization/colorAndIcons/cursorTheme"
            weight: 1
            OceanUIObject {
                name: "cursorThemeSelect"
                parentName: "personalization/colorAndIcons/cursorTheme/cursorThemeSelect"
                weight: 1
                pageType: OceanUIObject.Item
                page: IconThemeGridView {
                    model: oceanuiData.cursorThemeViewModel
                    onRequsetSetTheme: (id)=> {
                                           oceanuiData.worker.setCursorTheme(id)
                                       }
                }
            }
        }
    }
}
