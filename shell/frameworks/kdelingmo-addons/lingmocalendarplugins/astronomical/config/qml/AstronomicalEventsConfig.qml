/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.lingmocalendar.astronomicaleventsconfig 1.0
import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: configPage

    // expected API
    signal configurationChanged

    // expected API
    function saveConfig()
    {
        configStorage.isLunarPhaseShown = showLunarPhasesCheckBox.checked;
        configStorage.isSeasonShown = showSeasonsCheckBox.checked;

        configStorage.save();
    }

    LingmoUI.FormLayout {

        ConfigStorage {
            id: configStorage
        }

        QQC2.CheckBox {
            id: showLunarPhasesCheckBox

            LingmoUI.FormData.label: i18nd("lingmo_calendar_astronomicalevents", "Show:")

            checked: configStorage.isLunarPhaseShown
            text: i18ndc("lingmo_calendar_astronomicalevents", "@option:check", "Lunar phases")
            onCheckedChanged: configPage.configurationChanged();
        }

        QQC2.CheckBox {
            id: showSeasonsCheckBox

            checked: configStorage.isSeasonShown
            text: i18ndc("lingmo_calendar_astronomicalevents", "@option:check", "Astronomical seasons (solstices and equinoxes)")
            onCheckedChanged: configPage.configurationChanged();
        }
    }
}
