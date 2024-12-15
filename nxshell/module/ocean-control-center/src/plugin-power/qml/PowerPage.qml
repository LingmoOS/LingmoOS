// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "power/onPower"
        displayName: qsTr("Screen and Suspend")
        visible: oceanuiData.platformName() !== "wayland"
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "turnOffTheMonitorAfterGroup"
        parentName: "power/onPower"
        weight: 100
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "turnOffTheMonitorAfter"
            parentName: "power/onPower/turnOffTheMonitorAfterGroup"
            displayName: qsTr("Turn off the monitor after")
            weight: 1
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

                CustomTipsSlider {
                    dataMap: oceanuiData.model.linePowerScreenBlackDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.screenBlackDelayOnPower)
                    slider.onValueChanged: {
                        oceanuiData.worker.setScreenBlackDelayOnPower(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "computerSuspendsAfterGroup"
        parentName: "power/onPower"
        weight: 200
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "computerSuspendsAfter"
            parentName: "power/onPower/computerSuspendsAfterGroup"
            displayName: qsTr("Computer suspends after")
            weight: 1
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

                CustomTipsSlider {
                    dataMap: oceanuiData.model.linePowerSleepDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.sleepDelayOnPower)
                    slider.onValueChanged: {
                        oceanuiData.worker.setSleepDelayOnPower(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "lockScreenAfterGroup"
        parentName: "power/onPower"
        weight: 300
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "lockScreenAfter"
            parentName: "power/onPower/lockScreenAfterGroup"
            displayName: qsTr("Lock screen after")
            weight: 1
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
                CustomTipsSlider {
                    dataMap: oceanuiData.model.linePowerLockDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.powerLockScreenDelay)
                    slider.onValueChanged: {
                        oceanuiData.worker.setLockScreenDelayOnPower(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "powerButtonGroup"
        parentName: "power/onPower"
        weight: 400
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "whenTheLidIsClosed"
            parentName: "power/onPower/powerButtonGroup"
            displayName: qsTr("When the lid is closed")
            visible: oceanuiData.model.lidPresent
            weight: 1
            pageType: OceanUIObject.Editor
            page: CustomComboBox {
                textRole: "text"
                enableRole: "enable"
                visibleRole: "visible"
                width: 100
                model: oceanuiData.powerLidModel
                currentIndex: model.indexOfKey(oceanuiData.model.linePowerLidClosedAction)

                onCurrentIndexChanged: {
                    oceanuiData.worker.setLinePowerLidClosedAction(model.keyOfIndex(currentIndex))
                }
            }
        }
        OceanUIObject {
            name: "whenThePowerButtonIsPressed"
            parentName: "power/onPower/powerButtonGroup"
            displayName: qsTr("When the power button is pressed")
            weight: 2
            pageType: OceanUIObject.Editor
            visible: oceanuiData.platformName() !== "wayland"
            page: CustomComboBox {
                textRole: "text"
                enableRole: "enable"
                visibleRole: "visible"
                width: 100
                model: oceanuiData.powerPressModel
                currentIndex: model.indexOfKey(oceanuiData.model.linePowerPressPowerBtnAction)

                onCurrentIndexChanged: {
                    oceanuiData.worker.setLinePowerPressPowerBtnAction(model.keyOfIndex(currentIndex))
                }
            }
        }
    }
}
