// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils
import org.kde.lingmouiaddons.formcard as FormCard

SimpleKCM {
    id: root

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    data: [
        LingmoNM.Handler {
            id: handler
        },

        LingmoNM.WirelessStatus {
            id: wirelessStatus
        },

        LingmoUI.PromptDialog {
            id: hotspotDialog
            title: i18n("Configure Hotspot")
            standardButtons: LingmoUI.PromptDialog.Save | LingmoUI.PromptDialog.Cancel

            onOpened: {
                hotspotSsidField.text = LingmoNM.Configuration.hotspotName;
                hotspotPasswordField.text = LingmoNM.Configuration.hotspotPassword;
            }

            onAccepted: {
                LingmoNM.Configuration.hotspotName = hotspotSsidField.text;
                LingmoNM.Configuration.hotspotPassword = hotspotPasswordField.text;

                // these properties need to be manually updated since they're not NOTIFYable
                hotspotSSIDText.description = LingmoNM.Configuration.hotspotName;
                hotspotPasswordText.description = LingmoNM.Configuration.hotspotPassword;
            }

            ColumnLayout {
                Controls.Label {
                    text: i18n('Hotspot SSID:')
                }
                Controls.TextField {
                    Layout.fillWidth: true
                    id: hotspotSsidField
                }
                Controls.Label {
                    text: i18n('Hotspot Password:')
                }
                Controls.TextField {
                    Layout.fillWidth: true
                    id: hotspotPasswordField
                }
            }
        }
    ]

    ColumnLayout {
        spacing: 0

        FormCard.FormCard {
            Layout.topMargin: LingmoUI.Units.gridUnit

            FormCard.FormSwitchDelegate {
                id: hotspotToggle
                text: i18n("Hotspot")
                description: i18n("Share your internet connection with other devices as a Wi-Fi network.");

                checked: wirelessStatus.hotspotSSID.length !== 0

                onToggled: {
                    if (hotspotToggle.checked) {
                        handler.createHotspot();
                    } else {
                        handler.stopHotspot();
                    }
                }
            }
        }

        FormCard.FormHeader {
            title: i18n("Settings")
        }

        FormCard.FormCard {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            Layout.bottomMargin: LingmoUI.Units.largeSpacing

            FormCard.FormTextDelegate {
                id: hotspotSSIDText
                enabled: !hotspotToggle.checked
                text: i18n("Hotspot SSID")
                description: LingmoNM.Configuration.hotspotName
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                id: hotspotPasswordText
                enabled: !hotspotToggle.checked
                text: i18n("Hotspot Password")
                description: LingmoNM.Configuration.hotspotPassword
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormButtonDelegate {
                enabled: !hotspotToggle.checked
                text: i18n('Configure')
                onClicked: hotspotDialog.open()
            }
        }
    }
}
