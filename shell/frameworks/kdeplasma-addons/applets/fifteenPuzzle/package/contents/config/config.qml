/*
 * SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "Appearance")
         icon: "preferences-desktop-color"
         source: "configAppearance.qml"
    }
}
