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
        parentName: "power/onBattery"
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
        parentName: "power/onBattery"
        weight: 100
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "turnOffTheMonitorAfter"
            parentName: "power/onBattery/turnOffTheMonitorAfterGroup"
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
                    dataMap: oceanuiData.model.batteryScreenBlackDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.screenBlackDelayOnBattery)
                    slider.onValueChanged: {
                        oceanuiData.worker.setScreenBlackDelayOnBattery(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "lockScreenAfterGroup"
        parentName: "power/onBattery"
        weight: 200
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "lockScreenAfter"
            parentName: "power/onBattery/lockScreenAfterGroup"
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
                    dataMap: oceanuiData.model.batteryLockDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.batteryLockScreenDelay)
                    slider.onValueChanged: {
                        oceanuiData.worker.setLockScreenDelayOnBattery(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "computerSuspendsAfterGroup"
        parentName: "power/onBattery"
        weight: 300
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "computerSuspendsAfter"
            parentName: "power/onBattery/computerSuspendsAfterGroup"
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
                    dataMap: oceanuiData.model.batterySleepDelayModel
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.value: oceanuiData.indexByValueOnMap(dataMap, oceanuiData.model.sleepDelayOnBattery)
                    slider.onValueChanged: {
                        oceanuiData.worker.setSleepDelayOnBattery(dataMap[slider.value].value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "powerButtonGroup"
        parentName: "power/onBattery"
        weight: 400
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "whenTheLidIsClosed"
            parentName: "power/onBattery/powerButtonGroup"
            displayName: qsTr("When the lid is closed")
            visible: oceanuiData.model.lidPresent
            weight: 1
            pageType: OceanUIObject.Editor
            page: CustomComboBox {
                textRole: "text"
                enableRole: "enable"
                visibleRole: "visible"
                width: 100
                model: oceanuiData.batteryLidModel
                currentIndex: model.indexOfKey(oceanuiData.model.batteryLidClosedAction)

                onCurrentIndexChanged: {
                    oceanuiData.worker.setBatteryLidClosedAction(model.keyOfIndex(currentIndex))
                }
            }
        }
        OceanUIObject {
            name: "whenThePowerButtonIsPressed"
            parentName: "power/onBattery/powerButtonGroup"
            displayName: qsTr("When the power button is pressed")
            weight: 2
            pageType: OceanUIObject.Editor
            page: CustomComboBox {
                textRole: "text"
                enableRole: "enable"
                visibleRole: "visible"
                width: 100
                model: oceanuiData.batteryPressModel
                currentIndex: model.indexOfKey(oceanuiData.model.batteryPressPowerBtnAction)

                onCurrentIndexChanged: {
                    oceanuiData.worker.setBatteryPressPowerBtnAction(model.keyOfIndex(currentIndex))
                }
            }
        }
    }

    OceanUIObject {
        name: "lowBatteryTitle"
        parentName: "power/onBattery"
        displayName: qsTr("Low Battery")
        weight: 500
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
        name: "lowBatteryNotificationGroup"
        parentName: "power/onBattery"
        weight: 600
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "lowBatteryNotification"
            parentName: "power/onBattery/lowBatteryNotificationGroup"
            displayName: qsTr("Low battery notification")
            weight: 1
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                width: 100
                textRole: "text"
                flat: true
                currentIndex: oceanuiData.indexByValueOnModel(model, oceanuiData.model.lowPowerNotifyThreshold)
                model: ListModel {
                    ListElement { text: qsTr("Disbale"); value: 0 }
                    ListElement { text: "10%"; value: 10 }
                    ListElement { text: "15%"; value: 15 }
                    ListElement { text: "20%"; value: 20 }
                    ListElement { text: "25%"; value: 25 }
                }

                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    if (selectedValue === 0) {
                        oceanuiData.worker.setLowPowerNotifyEnable(false)
                        oceanuiData.worker.setLowPowerNotifyThreshold(selectedValue)
                    } else {
                        oceanuiData.worker.setLowPowerNotifyEnable(true)
                        oceanuiData.worker.setLowPowerNotifyThreshold(selectedValue)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "lowBatteryOperatorGroup"
        parentName: "power/onBattery"
        weight: 700
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "lowBatteryOperator"
            parentName: "power/onBattery/lowBatteryOperatorGroup"
            displayName: qsTr("Low battery operation")
            weight: 1
            pageType: OceanUIObject.Editor
            page: CustomComboBox {
                textRole: "text"
                valueRole: "value"
                flat: true
                currentIndex: indexByValue(oceanuiData.model.lowPowerAction)
                model: ListModel {
                    ListElement { text: qsTr("Auto Hibernate"); value: 1 }
                    ListElement { text: qsTr("Auto suspend"); value: 0 }
                }
                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    oceanuiData.worker.setLowPowerAction(selectedValue)
                }
            }
        }
        OceanUIObject {
            name: "lowBatteryThreshold"
            parentName: "power/onBattery/lowBatteryOperatorGroup"
            displayName: qsTr("Low battery threshold")
            weight: 2
            pageType: OceanUIObject.Editor
            page: CustomComboBox {
                textRole: "text"
                valueRole: "value"
                currentIndex: indexByValue(oceanuiData.model.lowPowerAutoSleepThreshold)
                model: ListModel {
                    ListElement { text: "1%"; value: 1 }
                    ListElement { text: "2%"; value: 2 }
                    ListElement { text: "3%"; value: 3 }
                    ListElement { text: "4%"; value: 4 }
                    ListElement { text: "5%"; value: 5 }
                    ListElement { text: "6%"; value: 6 }
                    ListElement { text: "7%"; value: 7 }
                    ListElement { text: "8%"; value: 8 }
                    ListElement { text: "9%"; value: 9 }
                }
                flat: true

                onCurrentIndexChanged: {
                    var selectedValue = model.get(currentIndex).value;
                    oceanuiData.worker.setLowPowerAutoSleepThreshold(selectedValue)
                }
            }
        }
    }

    OceanUIObject {
        name: "batteryManagementTitle"
        parentName: "power/onBattery"
        displayName: qsTr("Battery Management")
        weight: 800
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
        name: "batteryManagementGroup"
        parentName: "power/onBattery"
        weight: 900
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "displayRemainingUsingAndChargingTime"
            parentName: "power/onBattery/batteryManagementGroup"
            displayName: qsTr("Display remaining using and charging time")
            weight: 1
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.showBatteryTimeToFull
                onCheckedChanged: {
                    oceanuiData.worker.setShowBatteryTimeToFull(checked)
                }
            }
        }
        OceanUIObject {
            name: "maximumCapacity"
            parentName: "power/onBattery/batteryManagementGroup"
            displayName: qsTr("Maximum capacity")
            weight: 2
            pageType: OceanUIObject.Editor
            page: Label {
                    Layout.leftMargin: 10
                    text: oceanuiData.model.batteryCapacity + "%"
                }
        }
    }
}
