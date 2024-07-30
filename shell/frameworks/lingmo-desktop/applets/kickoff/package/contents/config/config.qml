/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.lingmo.configuration 2.0 as LingmoConfig

LingmoConfig.ConfigModel {
    LingmoConfig.ConfigCategory {
        name: i18n("General")
        icon: "preferences-desktop-lingmo"
        source: "ConfigGeneral.qml"
    }
}
