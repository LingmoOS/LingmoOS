//SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUITitleObject {
        name: "taskBarTitle"
        weight: 500
        parentName: "personalization/dock"
        displayName: qsTr("Dock")
    }

    OceanUIObject {
        name: "taskBarModeGroup"
        parentName: "personalization/dock"
        weight: 600
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "dockmode"
            parentName: "personalization/dock/taskBarModeGroup"
            displayName: qsTr("Mode")
            weight: 10
            pageType: OceanUIObject.Item
            page: ColumnLayout {
                Label {
                    Layout.topMargin: 10
                    font: D.DTK.fontManager.t7
                    text: oceanuiObj.displayName
                    Layout.leftMargin: 10
                }
                GridLayout {
                    id: modeLayout
                    rows: 1
                    Layout.bottomMargin: 10

                    ListModel {
                        id: modeData
                        ListElement { text: qsTr("Classic Mode"); icon: "effcient_left"; value: 1 }
                        ListElement { text: qsTr("Centered Mode"); icon: "effcient_center"; value: 0 }
                        // ListElement { text: qsTr("Fashion Mode"); icon: "fashion"; value: 2 }
                    }

                    Repeater {
                        model: modeData
                        ColumnLayout {
                            Item {
                                id: borderRect
                                Layout.preferredWidth: 206
                                Layout.preferredHeight: 100
                                Layout.alignment: Qt.AlignHCenter

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 12
                                    color: "transparent"
                                    border.width: 2
                                    border.color: D.DTK.platformTheme.activeColor
                                    visible: oceanuiData.dockInter.DisplayMode === model.value
                                }

                                D.DciIcon {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    sourceSize: Qt.size(width, height)
                                    name: model.icon
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        oceanuiData.dockInter.setDisplayMode(model.value)
                                    }
                                }
                            }

                            Text {
                                text: model.text
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: oceanuiData.dockInter.DisplayMode === model.value ? 
                                    D.DTK.platformTheme.activeColor : this.palette.windowText
                            }
                        }
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "dockSettingsGroup"
        parentName: "personalization/dock"
        weight: 700
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "docksize"
            parentName: "personalization/dock/dockSettingsGroup"
            displayName: qsTr("Dock size")
            weight: 10
            pageType: OceanUIObject.Editor
            page: RowLayout {
                spacing: 10
                Label {
                    Layout.alignment: Qt.AlignVCenter
                    font: D.DTK.fontManager.t7
                    text: qsTr("Small")
                }
                D.Slider {
                    Layout.alignment: Qt.AlignVCenter
                    id: balanceSlider
                    handleType: Slider.HandleType.ArrowBottom
                    implicitHeight: 24
                    highlightedPassedGroove: true
                    stepSize: 1
                    value: oceanuiData.dockInter.DisplayMode === 2 ? oceanuiData.dockInter.WindowSizeFashion : oceanuiData.dockInter.WindowSizeEfficient 
                    from: 37
                    to: 100

                    onValueChanged: {
                        oceanuiData.dockInter.resizeDock(value, true)
                    }

                    onPressedChanged: {
                        oceanuiData.dockInter.resizeDock(value, pressed)
                    }
                }
                D.Label {
                    Layout.alignment: Qt.AlignVCenter
                    font: D.DTK.fontManager.t7
                    text: qsTr("Large")
                }
            }
        }

        OceanUIObject {
            name: "positionInScreen"
            parentName: "personalization/dock/dockSettingsGroup"
            displayName: qsTr("Position on the screen")
            weight: 100
            pageType: OceanUIObject.Editor
            page: CustomComBobox {
                flat: true
                model: alignModel
                currentIndex: indexByValue(oceanuiData.dockInter.Position)

                ListModel {
                    id: alignModel
                    ListElement { text: qsTr("Top"); value: 0 }
                    ListElement { text: qsTr("Bottom"); value: 2 }
                    ListElement { text: qsTr("Left"); value: 3 }
                    ListElement { text: qsTr("Right"); value: 1 }
                }

                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    oceanuiData.dockInter.setPosition(selectedValue)
                }
            }
        }

        OceanUIObject {
            name: "positionInScreen"
            parentName: "personalization/dock/dockSettingsGroup"
            displayName: qsTr("Status")
            weight: 200
            pageType: OceanUIObject.Editor
            page: CustomComBobox {
                flat: true
                model: hideModel
                currentIndex: indexByValue(oceanuiData.dockInter.HideMode)

                ListModel {
                    id: hideModel
                    ListElement { text: qsTr("Keep shown"); value: 0 }
                    ListElement { text: qsTr("Keep hioceann"); value: 1 }
                    ListElement { text: qsTr("Smart hide"); value: 2 }
                }

                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    oceanuiData.dockInter.setHideMode(selectedValue)
                }
            }
        }
    }

    OceanUIObject {
        name: "multiscreenGroup"
        parentName: "personalization/dock"
        weight: 800
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "multiscreenItem"
            parentName: "personalization/dock/multiscreenGroup"
            displayName: qsTr("Multiple Displays")
            description: qsTr("Set the position of the taskbar on the screen")
            visible: Qt.application.screens.length > 1
            weight: 10
            pageType: OceanUIObject.Editor
            page: CustomComBobox {
                flat: true
                model: showModeModel
                currentIndex: indexByValue(oceanuiData.dockInter.showInPrimary)

                ListModel {
                    id: showModeModel
                    ListElement { text: qsTr("Only on main"); value: true }
                    ListElement { text: qsTr("On screen where the cursor is"); value: false }
                }

                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    oceanuiData.dockInter.setShowInPrimary(selectedValue)
                }
            }
        }
    }

    OceanUIObject {
        name: "pluginArea"
        weight:900
        icon: "plugin"
        parentName: "personalization/dock"
        displayName: qsTr("Plugin Area")
        description: qsTr("Select which icons appear in the Dock")

        PluginArea {}
    }
}
