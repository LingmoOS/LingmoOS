// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmouiaddons.formcard 1 as FormCard
import org.kde.lingmo.mm as LingmoMM

Item {
    id: root
    property string name: i18n("Cellular")

    readonly property real cardWidth: Math.min(LingmoUI.Units.gridUnit * 30, root.width - LingmoUI.Units.gridUnit * 2)

    function toggleMobileData() {
        if (LingmoMM.SignalIndicator.needsAPNAdded || !LingmoMM.SignalIndicator.mobileDataSupported) {
            // open settings if unable to toggle mobile data
            MobileShell.ShellUtil.executeCommand("lingmo-open-settings kcm_cellular_network");
        } else {
            LingmoMM.SignalIndicator.mobileDataEnabled = !LingmoMM.SignalIndicator.mobileDataEnabled;
        }
    }

    EditProfileDialog {
        id: profileDialog
        profile: null
    }

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: LingmoUI.Units.gridUnit
            bottomMargin: LingmoUI.Units.largeSpacing
        }
        width: root.width
        spacing: LingmoUI.Units.gridUnit

        Label {
            Layout.leftMargin: LingmoUI.Units.gridUnit
            Layout.rightMargin: LingmoUI.Units.gridUnit
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true

            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: {
                if (!LingmoMM.SignalIndicator.modemAvailable) {
                    return i18n("Your device does not have a modem available.");
                } else if (LingmoMM.SignalIndicator.needsAPNAdded) {
                    return i18n("Please configure your APN below for mobile data, further information will be available with your carrier.");
                } else if (LingmoMM.SignalIndicator.mobileDataSupported) {
                    return i18n("You are connected to the mobile network.");
                } else if (LingmoMM.SignalIndicator.simEmpty) {
                    return i18n("Please insert a SIM card into your device.");
                } else {
                    return i18n("Your device does not have a modem available.");
                }
            }
        }

        FormCard.FormCard {
            visible: LingmoMM.SignalIndicator.modemAvailable && LingmoMM.SignalIndicator.mobileDataSupported
            maximumWidth: root.cardWidth

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            FormCard.FormSwitchDelegate {
                text: i18n("Mobile Data")
                checked: LingmoMM.SignalIndicator.mobileDataEnabled
                onCheckedChanged: {
                    if (checked !== LingmoMM.SignalIndicator.mobileDataEnabled) {
                        root.toggleMobileData();
                    }
                }
            }
        }

        FormCard.FormCard {
            visible: LingmoMM.SignalIndicator.modemAvailable && !LingmoMM.SignalIndicator.simEmpty
            maximumWidth: root.cardWidth

            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            ListView {
                id: listView
                currentIndex: -1
                clip: true

                Layout.fillWidth: true
                Layout.fillHeight: true

                model: LingmoMM.SignalIndicator.profiles

                delegate: FormCard.FormRadioDelegate {
                    width: listView.width
                    text: modelData.name
                    description: modelData.apn
                    checked: modem.activeConnectionUni == modelData.connectionUni

                    onCheckedChanged: {
                        if (checked) {
                            LingmoMM.SignalIndicator.activateProfile(modelData.connectionUni);
                            checked = Qt.binding(() => { return modem.activeConnectionUni == modelData.connectionUni });
                        }
                    }

                    trailing: RowLayout {
                        ToolButton {
                            icon.name: "entry-edit"
                            text: i18n("Edit")
                            onClicked: {
                                profileDialog.profile = modelData;
                                profileDialog.open();
                            }
                        }
                        ToolButton {
                            icon.name: "delete"
                            text: i18n("Delete")
                            onClicked: LingmoMM.SignalIndicator.removeProfile(modelData.connectionUni)
                        }
                    }
                }
            }

            FormCard.FormButtonDelegate {
                icon.name: "list-add"
                text: i18n("Add APN")
                onClicked: {
                    profileDialog.profile = null;
                    profileDialog.open();
                }
            }
        }
    }
}
