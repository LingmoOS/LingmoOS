/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.plasmacalendar.astronomicaleventsconfig 1.0
import org.kde.kirigami 2.5 as Kirigami
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

    Kirigami.FormLayout {

        ConfigStorage {
            id: configStorage
        }

        QQC2.CheckBox {
            id: showLunarPhasesCheckBox

            Kirigami.FormData.label: i18nd("plasma_calendar_astronomicalevents", "Show:")

            checked: configStorage.isLunarPhaseShown
            text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Lunar phases")
            onCheckedChanged: configPage.configurationChanged();
        }

        QQC2.CheckBox {
            id: showSeasonsCheckBox

            checked: configStorage.isSeasonShown
            text: i18ndc("plasma_calendar_astronomicalevents", "@option:check", "Astronomical seasons (solstices and equinoxes)")
            onCheckedChanged: configPage.configurationChanged();
        }
    }
}
