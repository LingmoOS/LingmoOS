/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

QQC2.ApplicationWindow {
    width: 640
    height: 480
    visible: true

    QQC2.SwipeView {
        id: swipeView

        anchors.fill: parent
        currentIndex: navTabBar.currentIndex

        QQC2.Page {
            contentItem: QQC2.Label {
                text: "page1"
                horizontalAlignment: Text.AlignHCenter
            }
        }
        QQC2.Page {
            contentItem: QQC2.Label {
                text: "page2"
                horizontalAlignment: Text.AlignHCenter
            }
        }
        QQC2.Page {
            contentItem: QQC2.Label {
                text: "page3"
                horizontalAlignment: Text.AlignHCenter
            }
        }
        QQC2.Page {
            contentItem: QQC2.Label {
                text: "page4"
                horizontalAlignment: Text.AlignHCenter
            }
        }
        onCurrentIndexChanged: navTabBar.currentIndex = swipeView.currentIndex
    }

    footer: LingmoUI.NavigationTabBar {
        id: navTabBar

        currentIndex: swipeView.currentIndex

        LingmoUI.NavigationTabButton {
            visible: true
            width: navTabBar.buttonWidth
            icon.name: "document-save"
            text: `test ${tabIndex + 1}`
            QQC2.ButtonGroup.group: navTabBar.tabGroup
        }
        LingmoUI.NavigationTabButton {
            visible: false
            width: navTabBar.buttonWidth
            icon.name: "document-send"
            text: `test ${tabIndex + 1}`
            QQC2.ButtonGroup.group: navTabBar.tabGroup
        }
        actions: [
            LingmoUI.Action {
                visible: true
                icon.name: "edit-copy"
                icon.height: 32
                icon.width: 32
                text: "test 3"
                checked: true
            },
            LingmoUI.Action {
                visible: true
                icon.name: "edit-cut"
                text: "test 4"
                checkable: true
            },
            LingmoUI.Action {
                visible: false
                icon.name: "edit-paste"
                text: "test 5"
            },
            LingmoUI.Action {
                visible: true
                icon.source: "../logo.png"
                text: "test 6"
                checkable: true
            }
        ]
    }
}
