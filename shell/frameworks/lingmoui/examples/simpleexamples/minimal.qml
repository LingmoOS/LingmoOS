/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

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
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        LingmoUI.ScrollablePage {
            title: "Hello"
            Rectangle {
                anchors.fill: parent
            }
        }
    }
}
