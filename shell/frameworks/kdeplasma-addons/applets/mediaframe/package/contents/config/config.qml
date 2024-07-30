/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: i18nc("@title", "General")
         icon: "image"
         source: "ConfigGeneral.qml"
    }
    ConfigCategory {
         name: i18nc("@title", "Paths")
         icon: "folder"
         source: "ConfigPaths.qml"
    }
}
