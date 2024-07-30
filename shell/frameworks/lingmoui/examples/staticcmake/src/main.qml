/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    visible: true
    title: qsTr("Hello World")

    pageStack.initialPage: Page1 {}

    globalDrawer: LingmoUI.GlobalDrawer {
        actions: [
            LingmoUI.Action {
                text: "View"
                icon.name: "view-list-icons"
                LingmoUI.Action {
                    text: "action 1"
                }
                LingmoUI.Action {
                    text: "action 2"
                }
                LingmoUI.Action {
                    text: "action 3"
                }
            },
            LingmoUI.Action {
                text: "action 3"
            },
            LingmoUI.Action {
                text: "action 4"
            }
        ]
    }
}
