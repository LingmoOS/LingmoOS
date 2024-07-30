/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "Keys")
         icon: "input-caps-on"
         source: "configAppearance.qml"
    }
}
