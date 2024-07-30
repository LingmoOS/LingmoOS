/*
 *  SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "General")
         icon: "face-smile-big"
         source: "configGeneral.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Appearance")
         icon: "preferences-desktop-color"
         source: "configAppearance.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Advanced")
         icon: "system-run"
         source: "configAdvanced.qml"
    }
}
