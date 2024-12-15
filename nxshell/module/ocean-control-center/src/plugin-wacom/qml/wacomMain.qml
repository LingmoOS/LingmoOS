// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "wacomTitle"
        parentName: "wacom"
        displayName: qsTr("wacom")
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                height: contentHeight
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "mode"
        parentName: "wacom"
        displayName: qsTr("Wacom Mode")
        weight: 20
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: ComboBox {
            model: [qsTr("Pen Mode"), qsTr("Mouse Mode")]
            currentIndex: oceanuiData.CursorMode
            onCurrentIndexChanged: {
                oceanuiData.CursorMode = currentIndex
            }
        }
    }

    OceanUIObject {
        name: "pressure"
        parentName: "wacom"
        displayName: qsTr("Pressure Sensitivity")
        backgroundType: OceanUIObject.Normal
        weight: 30
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Layout.fillHeight: true
            Label {
                id: speedText
                Layout.topMargin: 10
                font: D.DTK.fontManager.t7
                text: oceanuiObj.displayName
                Layout.leftMargin: 10
            }
            D.TipsSlider {
                id: scrollSlider
                readonly property var tips: [qsTr("Light"), (""), (""), (""), (""), (""), ("Heavy")]
                Layout.preferredHeight: 90
                Layout.alignment: Qt.AlignCenter
                Layout.margins: 10
                Layout.fillWidth: true
                tickDirection: D.TipsSlider.TickDirection.Back
                slider.handleType: Slider.HandleType.ArrowBottom
                slider.value: oceanuiData.EraserPressureSensitive
                slider.from: 1
                slider.to: ticks.length
                slider.live: true
                slider.stepSize: 1
                slider.snapMode: Slider.SnapAlways
                ticks: [
                    D.SliderTipItem {
                        text: scrollSlider.tips[0]
                        highlight: scrollSlider.slider.value === 1
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[1]
                        highlight: scrollSlider.slider.value === 2
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[2]
                        highlight: scrollSlider.slider.value === 3
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[3]
                        highlight: scrollSlider.slider.value === 4
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[4]
                        highlight: scrollSlider.slider.value === 5
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[5]
                        highlight: scrollSlider.slider.value === 6
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[6]
                        highlight: scrollSlider.slider.value === 7
                    }
                ]
                slider.onValueChanged: {
                    oceanuiData.EraserPressureSensitive = slider.value
                }
            }
        }
    }
}
