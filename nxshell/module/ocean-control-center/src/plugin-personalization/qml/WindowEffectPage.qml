// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUITitleObject {
        name: "interfaceAndEffectTitle"
        parentName: "personalization/windowEffect"
        displayName: qsTr("Interface and Effects")
        visible: oceanuiData.platformName() !== "wayland"
        weight: 10
    }

    OceanUIObject {
        name: "interfaceAndEffect"
        parentName: "personalization/windowEffect"
        weight: 100
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: InterfaceEffectListview { }
    }

    OceanUITitleObject {
        name: "windowSettingsTitle"
        parentName: "personalization/windowEffect"
        displayName: qsTr("Window Settings")
        weight: 200
    }

    OceanUIObject {
        name: "windowSettingsGroup"
        parentName: "personalization/windowEffect"
        weight: 300
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "roundedEffect"
            parentName: "personalization/windowEffect/windowSettingsGroup"
            displayName: qsTr("Window rounded corners")
            visible: oceanuiData.model.windowEffectType < InterfaceEffectListview.WindowEffectType.Normal
            weight: 1
            pageType: OceanUIObject.Item
            page: ColumnLayout {
                anchors.fill: parent
                Label {
                    id: speedText
                    Layout.topMargin: 10
                    font: D.DTK.fontManager.t7
                    text: oceanuiObj.displayName
                    Layout.leftMargin: 10
                }

                ListView {
                    id: listview
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    Layout.margins: 10
                    clip: true
                    property var tips: [qsTr("None"), qsTr("Small"), qsTr("Medium"), qsTr("Large")]
                    property var icons: ["corner_none", "corner_small", "corner_middle", "corner_big"]

                    model: tips.length
                    orientation: ListView.Horizontal
                    layoutDirection: Qt.LeftToRight
                    spacing: 12
                    delegate: ColumnLayout {
                        id: layout
                        property bool checked : oceanuiData.model.windowRadius === 6 * index
                        width: 112
                        height: 104
                        Item {
                            Layout.preferredHeight: 77
                            Layout.fillWidth: true
                            Rectangle {
                                anchors.fill: parent
                                radius: 7
                                color: "transparent"
                                visible: layout.checked
                                border.width: 2
                                border.color: D.DTK.platformTheme.activeColor
                            }
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 4
                                color: Qt.rgba(0, 0, 0, 0.05)
                                radius: 7
                                D.DciIcon {
                                    sourceSize: Qt.size(parent.width, parent.height)
                                    name: listview.icons[index]
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    oceanuiData.worker.setWindowRadius(6 * index)
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: listview.tips[index]
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: layout.checked ? D.DTK.platformTheme.activeColor : this.palette.windowText
                        }
                    }
                }
            }
        }

        OceanUIObject {
            name: "enableTransparentWhenMoveWindow"
            parentName: "personalization/windowEffect/windowSettingsGroup"
            displayName: qsTr("Enable transparent effects when moving windows")
            visible: oceanuiData.model.windowEffectType < InterfaceEffectListview.WindowEffectType.Normal && oceanuiData.platformName() !== "wayland"
            weight: 2
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.isMoveWindow
                onCheckedChanged: {
                    oceanuiData.worker.setMovedWindowOpacity(checked)
                }
            }
        }

        OceanUIObject {
            name: "minimizeEffect"
            parentName: "personalization/windowEffect/windowSettingsGroup"
            displayName: qsTr("Window Minimize Effect")
            visible: oceanuiData.model.windowEffectType <= InterfaceEffectListview.WindowEffectType.Best && oceanuiData.platformName() !== "wayland"
            weight: 3
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                flat: true
                currentIndex: oceanuiData.model.miniEffect
                model: [qsTr("Scale"), qsTr("Magic Lamp")]
                onCurrentIndexChanged: {
                    oceanuiData.worker.setMiniEffect(currentIndex)
                }
            }
        }

    }

    OceanUIObject {
        name: "computerSuspendsAfter"
        parentName: "personalization/windowEffect"
        displayName: qsTr("Opacity")
        weight: 600
        visible: oceanuiData.model.windowEffectType < InterfaceEffectListview.WindowEffectType.Normal
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal
        page: ColumnLayout {
            Layout.fillHeight: true
            Label {
                Layout.topMargin: 10
                font: D.DTK.fontManager.t7
                text: oceanuiObj.displayName
                Layout.leftMargin: 10
            }

            D.TipsSlider {
                Layout.alignment: Qt.AlignCenter
                Layout.margins: 10
                Layout.fillWidth: true
                tickDirection: D.TipsSlider.TickDirection.Back
                slider.handleType: Slider.HandleType.ArrowBottom
                slider.from: 20
                slider.to: 100
                slider.live: true
                slider.stepSize: 1
                slider.snapMode: Slider.SnapAlways
                ticks: [
                    D.SliderTipItem {
                        text: qsTr("Low")
                    },
                    D.SliderTipItem {
                        text: ""
                    },
                    D.SliderTipItem {
                        text: qsTr("High")
                    }
                ]
                slider.onValueChanged: {
                    oceanuiData.worker.setOpacity(slider.value)
                }

                Component.onCompleted: {
                    slider.value = oceanuiData.model.opacity * 100
                }
            }
        }
    }

    OceanUIObject {
        id: scrollBarObject
        name: "scrollBar"
        property bool hasDBusProperty: false
        parentName: "personalization/windowEffect"
        displayName: qsTr("Scroll Bars")
        visible: oceanuiData.model.scrollBarPolicyConfig !== "Hioceann"
        weight: 700
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.ComboBox {
            flat: true
            enabled: oceanuiData.model.scrollBarPolicyConfig !== "Disabled"
            model: [qsTr("Show on scrolling"), qsTr("Keep shown")]
            currentIndex: {
                let policy = oceanuiData.model.scrollBarPolicy
                if (policy === Qt.ScrollBarAsNeeded) {
                    return 0
                } else {
                    return 1
                }
            }

            onCurrentIndexChanged: {
                if (currentIndex === 0) {
                    oceanuiData.worker.setScrollBarPolicy(Qt.ScrollBarAsNeeded)
                } else if(currentIndex === 1) {
                    oceanuiData.worker.setScrollBarPolicy(Qt.ScrollBarAlwaysOn)
                }
            }
        }
    }

    OceanUIObject {
        name: "compact"
        parentName: "personalization/windowEffect"
        displayName: qsTr("Compact Display")
        description: qsTr("If enabled, more content is displayed in the window.")
        visible: oceanuiData.model.compactDisplayConfig !== "Hioceann"
        weight: 700
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.Switch {
            enabled: oceanuiData.model.compactDisplayConfig !== "Disabled"
            checked: oceanuiData.model.compactDisplay
            onCheckedChanged: {
                oceanuiData.worker.setCompactDisplay(checked)
            }
        }
    }

    OceanUIObject {
        name: "titleBarHeight"
        parentName: "personalization/windowEffect"
        displayName: qsTr("Title Bar Height")
        description: qsTr("Only suitable for application window title bars drawn by the window manager.")
        weight: 700
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.ComboBox {
            flat: true
            currentIndex: indexOfValue(oceanuiData.model.titleBarHeight)
            model: [
                { text: qsTr("Extremely small"), value: 24 },
                { text: qsTr("Small"), value: 32 },
                { text: qsTr("Medium"), value: 40 },
                { text: qsTr("Large"), value: 50 }
            ]

            textRole: "text"
            valueRole: "value"

            onCurrentIndexChanged: {
                var selectedValue = model[currentIndex][valueRole]
                oceanuiData.worker.setDiabledCompactToTitleHeight()
                oceanuiData.worker.setTitleBarHeight(selectedValue)
            }

            function indexOfValue(value) {
                for (var i = 0; i < model.length; i++) {
                    if (model[i][valueRole] === value) {
                        return i
                    }
                }
                return -1
            }
        }
    }
}
