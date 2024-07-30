/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.configuration

ConfigModel {
    ConfigCategory {
         name: i18n("General")
         icon: "lingmo"
         source: "ConfigGeneral.qml"
    }
    ConfigCategory {
         name: i18n("Entries")
         icon: "preferences-desktop-notification"
         source: "ConfigEntries.qml"
    }
}
