/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: main

    pageStack.initialPage: LingmoUI.Page {
        QQC2.Button {
            text: "button"
            onClicked: menu.popup()
            QQC2.Menu {
                id: menu

                QQC2.MenuItem { text: "xxx" }
                QQC2.MenuItem { text: "xxx" }
                QQC2.Menu {
                    title: "yyy"
                    QQC2.MenuItem { text: "yyy" }
                    QQC2.MenuItem { text: "yyy" }
                }
            }
        }

        title: "aaaaaaaaaaaaaaa aaaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaaa"

        QQC2.ActionGroup {
            id: group
        }

        actions: [
            LingmoUI.Action {
                text: "submenus"
                icon.name: "kalgebra"

                LingmoUI.Action { text: "xxx"; onTriggered: console.log("xxx") }
                LingmoUI.Action { text: "xxx"; onTriggered: console.log("xxx") }
                LingmoUI.Action { text: "xxx"; onTriggered: console.log("xxx") }
                LingmoUI.Action {
                    text: "yyy"
                    LingmoUI.Action { text: "yyy" }
                    LingmoUI.Action { text: "yyy" }
                    LingmoUI.Action { text: "yyy" }
                    LingmoUI.Action { text: "yyy" }
                }
            },
            LingmoUI.Action {
                id: optionsAction
                text: "Options"
                icon.name: "kate"

                LingmoUI.Action {
                    QQC2.ActionGroup.group: group
                    text: "A"
                    checkable: true
                    checked: true
                }
                LingmoUI.Action {
                    QQC2.ActionGroup.group: group
                    text: "B"
                    checkable: true
                }
                LingmoUI.Action {
                    QQC2.ActionGroup.group: group
                    text: "C"
                    checkable: true
                }
            },
            LingmoUI.Action { text: "stuffing..." },
            LingmoUI.Action { text: "stuffing..." },
            LingmoUI.Action { text: "stuffing..." },
            LingmoUI.Action { text: "stuffing..." },
            LingmoUI.Action { text: "stuffing..." }
        ]
    }
}
