/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import org.kde.lingmo.plasmoid
import org.kde.lingmoui as LingmoUI

WallpaperItem {
    id: root

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.backgroundColor
    }

    LingmoUI.Heading {
        anchors.centerIn: parent
        level: 1
        text: wallpaper.configuration.DisplayText ||
              i18n("<Please configure a text to display>")
    }
}
