/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationItem {
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
        handleVisible: true
    }
    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer

        actions: (pageStack.currentItem as LingmoUI.Page)?.actions ?? []
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        LingmoUI.Page {
            title: "Hello"
            actions: [
                LingmoUI.Action {
                    text: "action 1"
                },
                LingmoUI.Action {
                    text: "action 2"
                }
            ]
            Rectangle {
                color: "#aaff7f"
                anchors.fill: parent
            }
        }
    }
}
