/*
    SPDX-FileCopyrightText: 2016 Chinmoy Ranjan Pradhan <chinmoyrp65@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.0
import QtQuick.Controls 2.5

import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.lingmo.plasmoid 2.0
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_compactView: compactViewRadioButton.checked

    LingmoUI.FormLayout {
        RadioButton {
            id: compactViewRadioButton
            text: i18n("Use single button for application menu")
        }

        RadioButton {
            id: fullViewRadioButton
            checked: !compactViewRadioButton.checked
            text: i18n("Show full application menu")
        }
    }
}
