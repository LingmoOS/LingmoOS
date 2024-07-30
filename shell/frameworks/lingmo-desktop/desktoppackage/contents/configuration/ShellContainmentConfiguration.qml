/*
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import org.kde.lingmoui 2.13 as LingmoUI

import "shellcontainmentconfiguration"

LingmoUI.AbstractApplicationWindow {
    id: root

    title: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Panel and Desktop Management")

    width: LingmoUI.Units.gridUnit * 40
    height: LingmoUI.Units.gridUnit * 32

    minimumWidth: LingmoUI.Units.gridUnit * 30
    minimumHeight: LingmoUI.Units.gridUnit * 25

    header: QQC2.ToolBar {
        anchors {
            left: parent.left
            right: parent.right
        }
        contentItem: QQC2.Label {
            Layout.fillWidth: parent
            text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "You can drag Panels and Desktops around to move them to different screens.")
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }

    footer: QQC2.Control {
        contentItem: QQC2.DialogButtonBox {
            QQC2.Button {
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Close")
                onClicked: Window.window.close()
            }
        }
        background: Item {
            // FIXME: automate that somehow?
            LingmoUI.Separator {
                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                }
                visible: mainPage.flickable.contentHeight > mainPage.flickable.height
            }
        }
    }

    LingmoUI.ScrollablePage {
        id: mainPage
        anchors.fill: parent

        leftPadding: 0
        topPadding: 0
        rightPadding: 0
        bottomPadding: 0

        Flow {
            id: mainGrid
            width: mainPage.flickable.width
            spacing: 0

            Repeater {
                id: repeater
                model: ShellContainmentModel

                delegate: Delegate {
                    viewPort: mainPage
                }
            }
        }
    }
}
