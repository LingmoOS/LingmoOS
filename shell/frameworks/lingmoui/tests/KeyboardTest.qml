/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: main

    Component {
        id: keyPage
        LingmoUI.Page {
            id: page

            // Don't remove, used in autotests
            readonly property alias lastKey: see.text

            Label {
                id: see
                anchors.centerIn: parent
                color: page.activeFocus ? LingmoUI.Theme.focusColor : LingmoUI.Theme.textColor
            }

            Keys.onPressed: event => {
                if (event.text) {
                    see.text = event.text
                } else {
                    see.text = event.key
                }
            }

            Keys.onEnterPressed: main.showPassiveNotification("page!")
        }
    }

    header: Label {
        padding: LingmoUI.Units.largeSpacing
        text: `focus: ${main.activeFocusItem}, current: ${main.pageStack.currentIndex}`
    }

    Component.onCompleted: {
        main.pageStack.push(keyPage)
        main.pageStack.push(keyPage)
    }
}
