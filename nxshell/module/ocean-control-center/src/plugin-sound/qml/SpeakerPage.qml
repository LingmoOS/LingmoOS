// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.lingmo.dtk 1.0

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    function toPercent(value: string) {
        return Number(value * 100).toFixed(0) + "%"
    }
    OceanUIObject {
        name: "output"
        parentName: "sound/outPut"
        displayName: qsTr("Output")
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "noOutput"
        parentName: "sound/outPut"
        weight: 20
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal
        visible: oceanuiData.model().outPutPortCombo.length === 0
        page: Column {
            width: parent.width
            Label {
                height: 100
                width: parent.width
                Layout.leftMargin: 10
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t4
                text: qsTr("No output device for sound found")
            }
        }
    }

    OceanUIObject {
        name: "outputGroup"
        parentName: "sound/outPut"
        weight: 30
        visible: oceanuiData.model().outPutPortCombo.length !== 0
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            height: implicitHeight + 10
            spacing: 0
        }
        OceanUIObject {
            name: "outputVolume"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Output Volume")
            weight: 10
            pageType: OceanUIObject.Editor
            page: RowLayout {
                Label {
                    font: DTK.fontManager.t7
                    text: root.toPercent(voiceTipsSlider.value)
                }
                IconButton {
                    Layout.alignment: Qt.AlignVCenter
                    icon.name: "qrc:/icons/lingmo/builtin/actions/oceanui_volume1_32px.svg"
                    icon.width: 24
                    icon.height: 24
                    implicitWidth: 24
                    background: Rectangle {
                        color: "transparent" // 背景透明
                        border.color: "transparent" // 边框透明
                        border.width: 0
                    }
                }
                Slider {
                    id: voiceTipsSlider
                    Layout.alignment: Qt.AlignVCenter
                    implicitHeight: 24
                    handleType: Slider.HandleType.NoArrowHorizontal
                    highlightedPassedGroove: true
                    value: oceanuiData.model().speakerVolume
                    to: oceanuiData.model().increaseVolume ? 1.5 : 1.0
                    onValueChanged: {
                        if (voiceTipsSlider.value != oceanuiData.model().speakerVolume) {
                            oceanuiData.worker().setSinkVolume(voiceTipsSlider.value)
                        }
                    }
                }
                IconButton {
                    icon.name: "qrc:/icons/lingmo/builtin/actions/oceanui_volume3_32px.svg"
                    icon.width: 24
                    icon.height: 24
                    implicitWidth: 24
                    background: Rectangle {
                        color: "transparent" // 背景透明
                        border.color: "transparent" // 边框透明
                        border.width: 0
                    }
                }
            }
        }
        OceanUIObject {
            name: "volumeEnhancement"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Volume Boost")
            description: qsTr("If the volume is louder than 100%, it may distort audio and be harmful to output devices")
            weight: 20
            pageType: OceanUIObject.Editor
            page: Switch {
                Layout.alignment: Qt.AlignRight
                checked: oceanuiData.model().increaseVolume

                onCheckedChanged: {
                    oceanuiData.worker().setIncreaseVolume(checked)
                }
            }
        }
        OceanUIObject {
            name: "volumeBalance"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Left Right Balance")
            weight: 30
            pageType: OceanUIObject.Editor
            page: RowLayout {
                Label {
                    Layout.alignment: Qt.AlignVCenter
                    font: DTK.fontManager.t7
                    Layout.topMargin: 2
                    text: qsTr("Left")
                }
                Slider {
                    id: balanceSlider
                    Layout.alignment: Qt.AlignVCenter
                    implicitHeight: 24
                    from: -1
                    handleType: Slider.HandleType.ArrowBottom
                    highlightedPassedGroove: true
                    stepSize: 0.00001
                    to: 1
                    value: oceanuiData.model().speakerBalance

                    onValueChanged: {
                        if (balanceSlider.value != oceanuiData.model().speakerBalance) {
                            oceanuiData.worker().setSinkBalance(balanceSlider.value)
                        }
                    }
                }
                Label {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.topMargin: 2
                    font: DTK.fontManager.t7
                    text: qsTr("Right")
                }
            }
        }
        OceanUIObject {
            name: "monoAudio"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Mono audio")
            description: qsTr("Merge left and right channels into a single channel")
            weight: 40
            pageType: OceanUIObject.Editor
            page: Switch {
                checked: oceanuiData.model().audioMono
                onCheckedChanged: {
                    oceanuiData.worker().setAudioMono(checked)
                }
            }
        }
        OceanUIObject {
            name: "plugAndUnplugManagement"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Auto pause")
            description: qsTr("Whether the audio will be automatically paused when the current audio device is unplugged")
            weight: 50
            pageType: OceanUIObject.Editor
            page: Switch {
                checked: oceanuiData.model().pausePlayer
                onCheckedChanged: {
                    oceanuiData.worker().setPausePlayer(checked)
                }
            }
        }
        OceanUIObject {
            name: "outputDevice"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Output Devices")
            weight: 60
            pageType: OceanUIObject.Editor
            page: ComboBox {
                id: outPutCombo
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 10
                flat: true
                currentIndex: oceanuiData.model().outPutPortComboIndex
                model: oceanuiData.model().outPutPortCombo
                enabled: oceanuiData.model().outPutPortComboEnable
                property bool isInitialized: false
                // 等待组件加载完成后，设置 isInitialized 为 true
                Component.onCompleted: {
                    console.log("outputDevice onCompleted:", isInitialized, oceanuiData.model().outPutPortComboEnable)
                    isInitialized = true
                }
                onCurrentIndexChanged: {
                    console.log("outputDevice Selected index:", currentIndex, isInitialized)
                    if (isInitialized && currentIndex !== oceanuiData.model().outPutPortComboIndex) {
                        oceanuiData.worker().setActivePort(currentIndex, 1)
                    }
                }
            }
        }

        OceanUIObject {
            name: "bluetoothMode"
            parentName: "sound/outPut/outputGroup"
            displayName: qsTr("Mode")
            weight: 60
            pageType: OceanUIObject.Editor
            visible: oceanuiData.model().showBluetoothMode
            page: ComboBox {
                id: bluetoothModeCombo
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 10
                flat: true
                model: oceanuiData.model().bluetoothModeOpts
                currentIndex: indexOfValue(oceanuiData.model().currentBluetoothAudioMode)
                property bool isInitialized: false
                // 等待组件加载完成后，设置 isInitialized 为 true
                Component.onCompleted: {
                    console.log("bluetoothModeCombo onCompleted:", isInitialized)
                    incrementCurrentIndex()
                    isInitialized = true
                }
                onCurrentIndexChanged: {
                    console.log("bluetoothModeCombo Selected index:", currentIndex, isInitialized)
                    if (isInitialized) {
                        oceanuiData.worker().setBluetoothMode(valueAt(currentIndex))
                    }
                }
            }
        }
    }
}
