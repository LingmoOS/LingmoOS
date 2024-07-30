/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: main

    pageStack.initialPage: LingmoUI.ScrollablePage {
        ListView {
            model: 10
            delegate: Rectangle {
                width: 100
                height: 30
                color: "white"
                border.color: ListView.isCurrentItem ? "#1EA8F7" : "transparent"
                border.width: 4
                radius: 4
            }
        }
    }
}
