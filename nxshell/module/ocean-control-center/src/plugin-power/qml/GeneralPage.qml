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
        name: "powerPlansTitle"
        parentName: "power/general"
        displayName: qsTr("Power Plans")
        weight: 10
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "powerPlans"
        parentName: "power/general"
        weight: 100
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: PowerPlansListview {
        }
    }

    OceanUIObject {
        name: "powerSavingSettingsTitle"
        parentName: "power/general"
        displayName: qsTr("Power Saving Settings")
        weight: 200
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "powerSavingSettingsGroup"
        parentName: "power/general"
        weight: 300
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "autoPowerSavingOnLowBattery"
            parentName: "power/general/powerSavingSettingsGroup"
            displayName: qsTr("Auto power saving on low battery")
            weight: 1
            visible: oceanuiData.model.haveBettary
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.powerSavingModeAutoWhenQuantifyLow
                onCheckedChanged: {
                    oceanuiData.worker.setPowerSavingModeAutoWhenQuantifyLow(checked)
                }
            }
        }

        OceanUIObject {
            name: "lowPowerThreshold"
            parentName: "power/general/powerSavingSettingsGroup"
            displayName: qsTr("Low battery threshold")
            weight: 2
            visible: oceanuiData.model.haveBettary && oceanuiData.model.powerSavingModeAutoWhenQuantifyLow
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                width: 100
                model: [ "10%", "20%", "30%", "40%", "50%" ]
                flat: true
                currentIndex: oceanuiData.model.powerSavingModeAutoBatteryPercentage / 10 - 1

                onCurrentIndexChanged: {
                    oceanuiData.worker.setPowerSavingModeAutoBatteryPercentage((currentIndex + 1) * 10)
                }
            }
        }
    }

    OceanUIObject {
        name: "savePowerGroup"
        parentName: "power/general"
        weight: 400
        pageType: OceanUIObject.Item
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "autoPowerSavingOnBattery"
            parentName: "power/general/savePowerGroup"
            displayName: qsTr("Auto power saving on battery")
            weight: 1
            visible: oceanuiData.model.haveBettary
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.autoPowerSaveMode
                onCheckedChanged: {
                    oceanuiData.worker.setPowerSavingModeAuto(checked)
                }
            }
        }

        OceanUIObject {
            name: "decreaseBrightness"
            parentName: "power/general/savePowerGroup"
            displayName: qsTr("Auto reduce screen brightness during power saving")
            weight: 2
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
                    readonly property var tips: [("10%"), ("20%"), ("30%"), ("40%")]
                    Layout.preferredHeight: 80
                    Layout.alignment: Qt.AlignCenter
                    Layout.margins: 10
                    Layout.fillWidth: true
                    slider.handleType: Slider.HandleType.ArrowBottom
                    slider.from: 10
                    slider.to: ticks.length * 10
                    slider.live: true
                    slider.stepSize: 10
                    slider.snapMode: Slider.SnapAlways
                    slider.value: oceanuiData.model.powerSavingModeLowerBrightnessThreshold
                    ticks: [
                        D.SliderTipItem { text: scrollSlider.tips[0]; highlight: scrollSlider.slider.value === 10 },
                        D.SliderTipItem { text: scrollSlider.tips[1]; highlight: scrollSlider.slider.value === 20 },
                        D.SliderTipItem { text: scrollSlider.tips[2]; highlight: scrollSlider.slider.value === 30 },
                        D.SliderTipItem { text: scrollSlider.tips[3]; highlight: scrollSlider.slider.value === 40 }
                    ]

                    slider.onValueChanged: {
                        oceanuiData.worker.setPowerSavingModeLowerBrightnessThreshold(slider.value)
                    }
                }
            }
        }
    }

    OceanUIObject {
        name: "wakeupSettingsTitle"
        parentName: "power/general"
        displayName: qsTr("Wakeup Settings")
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
        name: "wakeupSettingsGroup"
        parentName: "power/general"
        weight: 600
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "passwordIsRequiredToWakeUpTheComputer"
            parentName: "power/general/wakeupSettingsGroup"
            displayName: qsTr("Password is required to wake up the computer")
            weight: 1
            visible: oceanuiData.model.canSuspend && oceanuiData.model.isSuspend
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.sleepLock && !oceanuiData.model.isNoPasswdLogin
                enabled: !oceanuiData.model.isNoPasswdLogin
                onCheckedChanged: {
                    oceanuiData.worker.setSleepLock(checked)
                }
            }
        }

        OceanUIObject {
            name: "passwordIsRequiredToWakeUpTheMonitor"
            parentName: "power/general/wakeupSettingsGroup"
            displayName: qsTr("Password is required to wake up the monitor")
            weight: 2
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.screenBlackLock && !oceanuiData.model.isNoPasswdLogin
                enabled: !oceanuiData.model.isNoPasswdLogin
                onCheckedChanged: {
                    oceanuiData.worker.setScreenBlackLock(checked)
                }
            }
        }
    }

    OceanUIObject {
        name: "shutdownSettingTitle"
        parentName: "power/general"
        displayName: qsTr("Shutdown Settings")
        weight: 700
        visible: oceanuiData.model.enableScheduledShutdown !== "Hioceann" && oceanuiData.platformName() !== "wayland"
        enabled: oceanuiData.model.enableScheduledShutdown !== "Disabled"
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
        name: "shutdownGroup"
        parentName: "power/general"
        weight: 800
        pageType: OceanUIObject.Item
        visible: oceanuiData.model.enableScheduledShutdown !== "Hioceann" && oceanuiData.platformName() !== "wayland"
        enabled: oceanuiData.model.enableScheduledShutdown !== "Disabled"
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "timedPoweroff"
            parentName: "power/general/shutdownGroup"
            displayName: qsTr("Scheduled Shutdown")
            weight: 1
            pageType: OceanUIObject.Editor
            page: D.Switch {
                checked: oceanuiData.model.scheduledShutdownState
                onCheckedChanged: {
                    oceanuiData.worker.setScheduledShutdownState(checked)
                }
            }
        }

        OceanUIObject {
            name: "poweroffTime"
            parentName: "power/general/shutdownGroup"
            visible: oceanuiData.model.scheduledShutdownState
            displayName: qsTr("Time")
            weight: 2
            pageType: OceanUIObject.Editor
            page: RowLayout {
                OceanUITimeRange {
                    Layout.preferredWidth: 100
                    hour: oceanuiData.model.shutdownTime.split(':')[0]
                    minute: oceanuiData.model.shutdownTime.split(':')[1]
                    property string timeStr: `${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}`
                    onTimeStrChanged: {
                        oceanuiData.worker.setShutdownTime(timeStr)
                    }
                }
            }
        }

        OceanUIObject {
            name: "repeatDays"
            parentName: "power/general/shutdownGroup"
            visible: oceanuiData.model.scheduledShutdownState
            displayName: qsTr("Repeat")
            weight: 3
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                width: 100
                model: [ qsTr("Once"), qsTr("Every day"), qsTr("Working days"), qsTr("Custom Time") ]
                flat: true
                currentIndex: oceanuiData.model.shutdownRepetition
                onCurrentIndexChanged: {
                    oceanuiData.worker.setShutdownRepetition(currentIndex)
                    if (currentIndex === model.length - 1 && oceanuiData.model.customShutdownWeekDays.length === 0) {
                        repeatDaysEditObject.requestShowSelectDialog()
                    }
                }
            }
        }
        OceanUIObject {
            id: repeatDaysEditObject
            name: "repeatDaysEdit"
            parentName: "power/general/shutdownGroup"
            visible: oceanuiData.model.scheduledShutdownState && oceanuiData.model.shutdownRepetition === 3
            weight: 4
            pageType: OceanUIObject.Editor
            signal requestShowSelectDialog()
            page: RowLayout {
                Connections {
                    target: repeatDaysEditObject
                    function onRequestShowSelectDialog() {
                        selectDayDialog.show()
                    }
                }

                Label {
                    text: {
                        var str = ""
                        var days = oceanuiData.model.customShutdownWeekDays
                        for (var i = 0; i < days.length; i++) {
                            str += selectDayDialog.dateStr[days[i] - 1] + ", "
                        }
                        if (str.length > 1) {
                            str = str.slice(0, -2)
                        }
                        return str;
                    }
                }
                D.ToolButton {
                    icon.name: "action_edit"
                    icon.width: 12
                    icon.height: 12
                    implicitWidth: 24
                    implicitHeight: 24
                    background: Rectangle {
                        color: "transparent"
                        border.color: "transparent"
                        border.width: 0
                    }

                    onClicked: {
                        selectDayDialog.show()
                    }
                }
                D.DialogWindow {
                    id: selectDayDialog
                    width: 330
                    height: 420

                    // Copy is used here to prevent contamination of data in the original model when selecting items
                    property var selectedDays: oceanuiData.model.customShutdownWeekDays.length === 0 ? [1, 2, 3, 4, 5] : oceanuiData.model.customShutdownWeekDays.slice()
                    property var dateStr: [qsTr("Monday"), qsTr("Tuesday"), qsTr("Wednesday"), qsTr("Thursday"), qsTr("Friday"), qsTr("Saturday"), qsTr("Sunday")]
                    property var dayModel: generateDayModel()

                    function generateDayModel() {
                        var array = []
                        let weekBegin = oceanuiData.model.weekBegins
                        if (weekBegin < 1 || weekBegin > 7) {
                            // default is monday
                            weekBegin = 1
                        }
                        for (var i = oceanuiData.model.weekBegins; i <= 7; i++) {
                            array.push(i)
                        }
                        for (i = 1; i < oceanuiData.model.weekBegins; i++) {
                            array.push(i)
                        }
                        return array
                    }

                    // only accept close event , the app can quit normally
                    onClosing: function(close) {
                        close.accepted = true
                    }

                    ColumnLayout {
                        implicitWidth: parent.width
                        clip: true
                        Text {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Customize repetition time")
                        }
                        ListView {
                            Layout.fillWidth: true
                            height: contentHeight
                            clip: true
                            model: selectDayDialog.dayModel
                            spacing: 2
                            delegate: D.ItemDelegate {
                                width: ListView.view.width
                                leftPadding: 10
                                rightPadding: 10
                                implicitHeight: 40
                                checkable: false
                                corners: getCornersForBackground(index, 7)
                                cascadeSelected: true
                                text: selectDayDialog.dateStr[modelData - 1]
                                onClicked: handleSelected(modelData)
                                content: OceanUICheckIcon {
                                    checked: selectDayDialog.selectedDays.indexOf(modelData) !== -1
                                    onClicked: handleSelected(modelData)
                                }
                                background: OceanUIItemBackground {
                                    separatorVisible: true
                                }

                                function handleSelected(index) {
                                    if (selectDayDialog.selectedDays.indexOf(index) === -1) {
                                        selectDayDialog.selectedDays.push(index);
                                    } else {
                                        selectDayDialog.selectedDays.splice(selectDayDialog.selectedDays.indexOf(index), 1);
                                    }
                                    selectDayDialog.selectedDays = selectDayDialog.selectedDays.slice();
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 10
                            D.Button {
                                text: qsTr("Cancel")
                                onClicked: {
                                    selectDayDialog.close()
                                }
                            }
                            D.Button {
                                text: qsTr("Save")
                                onClicked: {
                                    var days = selectDayDialog.selectedDays.sort()
                                    console.log("Selected days: " + days);
                                    oceanuiData.worker.setCustomShutdownWeekDays(days)
                                    selectDayDialog.close()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
