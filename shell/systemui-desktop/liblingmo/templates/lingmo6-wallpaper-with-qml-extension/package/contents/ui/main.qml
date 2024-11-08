/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.plasmoid
import org.kde.lingmo.private.%{APPNAMELC} 1.0

WallpaperItem {
    id: root

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.backgroundColor
    }

    ColumnLayout {
        anchors.centerIn: parent

        LingmoUI.Heading {
            Layout.alignment: Qt.AlignCenter
            level: 1
            text: wallpaper.configuration.DisplayText ||
                  i18n("<Please configure a text to display>")
        }

        LingmoComponents.Label {
            Layout.alignment: Qt.AlignCenter
            text: HelloWorld.text
        }
    }
}
