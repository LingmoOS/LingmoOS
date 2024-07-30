// -*- coding: iso-8859-1 -*-
/*
 *   SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
 *   SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils
import org.kde.timesettings
import org.kde.lingmouiaddons.formcard 1 as FormCard
import org.kde.lingmouiaddons.delegates 1 as Delegates

SimpleKCM {
    id: timeModule

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    ColumnLayout {
        spacing: 0

        FormCard.FormHeader {
            title: i18n("Display")
        }

        FormCard.FormCard {
            FormCard.FormSwitchDelegate {
                id: hourFormatSwitch
                text: i18n("24-Hour Format")
                description: i18n("Whether to use a 24-hour format for clocks.")
                checked: kcm.twentyFour
                onCheckedChanged: {
                    kcm.twentyFour = checked
                }
            }

            FormCard.FormDelegateSeparator { above: hourFormatSwitch; below: timeZoneSelect }

            FormCard.FormButtonDelegate {
                id: timeZoneSelect
                text: i18n("Timezone")
                description: kcm.timeZone
                onClicked: timeZonePickerDialog.open()
            }
        }

        FormCard.FormHeader {
            title: i18n("Time and Date")
        }

        FormCard.FormCard {
            FormCard.FormSwitchDelegate {
                id: ntpCheckBox
                text: i18n("Automatic Time Synchronization")
                description: i18n("Whether to set the time automatically.")
                checked: kcm.useNtp
                onCheckedChanged: {
                    kcm.useNtp = checked
                    if (!checked) {
                        kcm.ntpServer = "";
                        kcm.saveTime();
                    }
                }
            }

            FormCard.FormDelegateSeparator { above: ntpCheckBox; below: timeSelect }

            FormCard.FormButtonDelegate {
                id: timeSelect
                enabled: !ntpCheckBox.checked
                icon.name: "clock"
                text: i18n("System Time")
                description: Qt.formatTime(kcm.currentTime, kcm.twentyFour ? 'hh:mm' : 'hh:mm ap')
                onClicked: timePickerDialog.open()
            }

            FormCard.FormDelegateSeparator { above: timeSelect; below: dateSelect }

            FormCard.FormButtonDelegate {
                id: dateSelect
                text: i18n("System Date")
                description: Qt.formatDate(kcm.currentDate, Locale.LongFormat)
                icon.name: "view-calendar"
                enabled: !ntpCheckBox.checked
                onClicked: datePickerDialog.open()
            }
        }
    }

    data: [
        LingmoUI.Dialog {
            id: timeZonePickerDialog
            title: i18nc("@title:window", "Pick Timezone")
            standardButtons: LingmoUI.Dialog.Ok | LingmoUI.Dialog.Cancel

            property string selectedTimeZoneId

            onOpened: {
                selectedTimeZoneId = kcm.timeZone;
            }

            onAccepted: {
                kcm.saveTimeZone(selectedTimeZoneId)
            }

            ListView {
                id: listView
                currentIndex: -1 // otherwise the vkbd will constantly open and close while typing
                headerPositioning: ListView.OverlayHeader
                implicitWidth: 18 * LingmoUI.Units.gridUnit
                implicitHeight: 18 * LingmoUI.Units.gridUnit

                header: Controls.Control {
                    z: 1

                    topPadding: LingmoUI.Units.smallSpacing
                    bottomPadding: 0
                    leftPadding: LingmoUI.Units.smallSpacing
                    rightPadding: LingmoUI.Units.smallSpacing

                    background: Rectangle {
                        color: LingmoUI.Theme.backgroundColor
                    }

                    contentItem: ColumnLayout {
                        spacing: LingmoUI.Units.smallSpacing
                        LingmoUI.SearchField {
                            Layout.fillWidth: true
                            onTextChanged: kcm.timeZonesModel.filterString = text
                        }
                        LingmoUI.Separator { Layout.fillWidth: true }
                    }
                }

                model: kcm.timeZonesModel
                delegate: Controls.RadioDelegate {
                    z: -1
                    width: ListView.view.width
                    checked: timeZonePickerDialog.selectedTimeZoneId == model.timeZoneId

                    text: {
                        if (model.region) {
                            return "%1 / %2".arg(model.region).arg(model.city);
                        } else {
                            return model.city;
                        }
                    }

                    onClicked: {
                        timeZonePickerDialog.selectedTimeZoneId = model.timeZoneId;
                        checked = Qt.binding(() => timeZonePickerDialog.selectedTimeZoneId == model.timeZoneId);
                        console.log(timeZonePickerDialog.selectedTimeZoneId + ' ' + model.timeZoneId + ' ' + (timeZonePickerDialog.selectedTimeZoneId == model.timeZoneId));
                    }
                }
            }
        },

        LingmoUI.PromptDialog {
            id: datePickerDialog
            title: i18n("Pick System Date")
            topPadding: LingmoUI.Units.largeSpacing
            bottomPadding: LingmoUI.Units.largeSpacing
            preferredWidth: LingmoUI.Units.gridUnit * 15
            preferredHeight: LingmoUI.Units.gridUnit * 13

            standardButtons: LingmoUI.Dialog.Save | LingmoUI.Dialog.Cancel

            onAccepted: {
                kcm.currentDate = datePicker.isoDate;
                kcm.saveTime();
            }

            onOpened: {
                let date = new Date(kcm.currentDate)
                datePicker.day = date.getDate();
                datePicker.month = date.getMonth() + 1;
                datePicker.year = date.getFullYear();
            }

            DatePicker {
                id: datePicker
                implicitHeight: LingmoUI.Units.gridUnit * 6

                Connections {
                    target: kcm
                    function onCurrentDateChanged() {
                        if (datePickerDialog.visible) {
                            return;
                        }

                        let date = new Date(kcm.currentDate);
                        datePicker.day = date.getDate();
                        datePicker.month = date.getMonth() + 1;
                        datePicker.year = date.getFullYear();
                    }
                }
            }
        },

        LingmoUI.PromptDialog {
            id: timePickerDialog
            title: i18n("Pick System Time")
            preferredWidth: LingmoUI.Units.gridUnit * 15
            topPadding: LingmoUI.Units.largeSpacing
            bottomPadding: LingmoUI.Units.largeSpacing

            standardButtons: LingmoUI.Dialog.Save | LingmoUI.Dialog.Cancel

            onAccepted: {
                kcm.currentTime = String(timePicker.hours).padStart(2, '0')
                    + ':'
                    + String(timePicker.minutes).padStart(2, '0')
                    + ':00';
                kcm.saveTime();
            }

            onOpened: {
                var date = new Date(kcm.currentTime);
                timePicker.hours = date.getHours();
                timePicker.minutes = date.getMinutes();
                console.log(date + ' ' + date.getHours() + date.getMinutes())
            }

            TimePicker {
                id: timePicker

                Connections {
                    target: kcm
                    function onCurrentTimeChanged() {
                        if (timePicker.userConfiguring) {
                            return;
                        }

                        var date = new Date(kcm.currentTime);
                        timePicker.hours = date.getHours();
                        timePicker.minutes = date.getMinutes();
                    }
                }
            }
        }
    ]
}
