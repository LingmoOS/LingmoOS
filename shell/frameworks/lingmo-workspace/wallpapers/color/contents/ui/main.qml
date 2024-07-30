/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid 2.0

WallpaperItem {
    id: root

    Rectangle {
        anchors.fill: parent
        color: root.configuration.Color

        Behavior on color {
            SequentialAnimation {
                ColorAnimation {
                    duration: LingmoUI.Units.longDuration
                }

                ScriptAction {
                    script: root.accentColorChanged()
                }
            }
        }
    }
}
