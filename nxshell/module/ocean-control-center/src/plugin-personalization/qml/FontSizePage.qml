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
    OceanUIObject {
        name: "fontSize"
        parentName: "personalization/font"
        displayName: qsTr("Size")
        backgroundType: OceanUIObject.Normal
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Layout.fillHeight: true
            Label {
                Layout.topMargin: 10
                font: D.DTK.fontManager.t7
                text: oceanuiObj.displayName
                Layout.leftMargin: 10
            }
            D.TipsSlider {
                id: fontSizeSlider
                readonly property var fontSizeModel: [11, 12, 13, 14, 15, 16, 18, 20]
                Layout.preferredHeight: 90
                Layout.alignment: Qt.AlignCenter
                Layout.margins: 10
                Layout.fillWidth: true
                tickDirection: D.TipsSlider.TickDirection.Back
                slider.handleType: Slider.HandleType.ArrowBottom
                slider.from: 0
                slider.to: fontSizeModel.length - 1
                slider.live: true
                slider.stepSize: 1
                slider.snapMode: Slider.SnapAlways
                ticks: [
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[0]; highlight: fontSizeSlider.slider.value === 0},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[1]; highlight: fontSizeSlider.slider.value === 1},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[2]; highlight: fontSizeSlider.slider.value === 2},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[3]; highlight: fontSizeSlider.slider.value === 3},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[4]; highlight: fontSizeSlider.slider.value === 4},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[5]; highlight: fontSizeSlider.slider.value === 5},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[6]; highlight: fontSizeSlider.slider.value === 6},
                    D.SliderTipItem {text: fontSizeSlider.fontSizeModel[7]; highlight: fontSizeSlider.slider.value === 7}
                ]
                slider.onValueChanged: {
                    oceanuiData.worker.setFontSize(fontSizeSlider.fontSizeModel[fontSizeSlider.slider.value])
                }

                Component.onCompleted: {
                    slider.value = fontSizeModel.indexOf(oceanuiData.model.fontSizeModel.size)
                }
            }
        }
    }

    OceanUIObject {
        name: "standardFont"
        parentName: "personalization/font"
        displayName: qsTr("Standard Font")
        weight: 100
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: FontCombobox {
            flat: true
            model: oceanuiData.model.standardFontModel.fontList
            textRole: "Name"
            valueRole: "Id"
            currentIndex: indexOfValue(oceanuiData.model.standardFontModel.fontName)
            onCurrentIndexChanged: {
                oceanuiData.worker.setDefault(model[currentIndex]);
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

    OceanUIObject {
        name: "monoFont"
        parentName: "personalization/font"
        displayName: qsTr("Monospaced Font")
        weight: 200
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: FontCombobox {
            flat: true
            model: oceanuiData.model.monoFontModel.fontList
            textRole: "Name"
            valueRole: "Id"
            currentIndex: indexOfValue(oceanuiData.model.monoFontModel.fontName)
            onCurrentIndexChanged: {
                oceanuiData.worker.setDefault(model[currentIndex]);
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
