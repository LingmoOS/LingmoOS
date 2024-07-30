/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    globalDrawer: LingmoUI.GlobalDrawer {
        actions: [
            LingmoUI.Action {
                text: "push"
                onTriggered: pageStack.push(secondPageComponent)
            },
            LingmoUI.Action {
                text: "pop"
                onTriggered: pageStack.pop()
            },
            LingmoUI.Action {
                text: "clear"
                onTriggered: pageStack.clear()
            },
            LingmoUI.Action {
                text: "replace"
                onTriggered: pageStack.replace(secondPageComponent)
            }
        ]
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        LingmoUI.Page {
            title: "First Page"
            Rectangle {
                anchors.fill: parent
                QQC2.Label {
                    text: "First Page"
                }
            }
        }
    }

    Component {
        id: secondPageComponent
        LingmoUI.Page {
            title: "Second Page"
            Rectangle {
                color: "red"
                anchors.fill: parent
                QQC2.Label {
                    text: "Second Page"
                }
            }
        }
    }
}
