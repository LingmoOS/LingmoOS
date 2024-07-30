// SPDX-FileCopyrightText: 2020-2023 Devin Lin <espidev@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12 as Controls

import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.lingmo.mm as LingmoMM

LingmoUI.Dialog {
    id: dialog
    title: i18n("Edit APN")
    clip: true

    property var profile

    standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

    onAccepted: {
        if (profile === null) { // create new profile
            LingmoMM.SignalIndicator.addProfile(profileName.text, profileApn.text, profileUsername.text, profilePassword.text, profileNetworkType.value);
        } else { // edit existing profile
            LingmoMM.SignalIndicator.updateProfile(profile.connectionUni, profileName.text, profileApn.text, profileUsername.text, profilePassword.text, profileNetworkType.value);
        }
    }
    preferredWidth: LingmoUI.Units.gridUnit * 20
    padding: LingmoUI.Units.gridUnit

    ColumnLayout {
        LingmoUI.FormLayout {
            Layout.fillWidth: true
            wideMode: false

            Controls.TextField {
                id: profileName
                LingmoUI.FormData.label: i18n("Name")
                text: profile !== null ? profile.name : ""
            }
            Controls.TextField {
                id: profileApn
                LingmoUI.FormData.label: i18n("APN")
                text: profile != null ? profile.apn : ""
            }
            Controls.TextField {
                id: profileUsername
                LingmoUI.FormData.label: i18n("Username")
                text: profile != null ? profile.user : ""
            }
            Controls.TextField {
                id: profilePassword
                LingmoUI.FormData.label: i18n("Password")
                text: profile != null ? profile.password : ""
            }
            Controls.ComboBox {
                id: profileNetworkType
                LingmoUI.FormData.label: i18n("Network type")
                model: [i18n("4G/3G/2G"), i18n("3G/2G"), i18n("2G"), i18n("Only 4G"), i18n("Only 3G"), i18n("Only 2G"), i18n("Any")]
                Component.onCompleted: if (profile !== null) {
                    currentIndex = indexOfValue(profile.networkType);
                }
            }
        }
    }
}
