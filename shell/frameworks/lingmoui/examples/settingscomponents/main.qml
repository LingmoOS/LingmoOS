/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    title: qsTr("Hello, World")

    globalDrawer: LingmoUI.GlobalDrawer {
        isMenu: !LingmoUI.isMobile
        actions: [
            LingmoUI.Action {
                text: qsTr("Settings")
                icon.name: "settings-configure"
                onTriggered: root.pageStack.pushDialogLayer(Qt.resolvedUrl("./SettingsPage.qml"), {
                    width: root.width
                }, {
                    title: qsTr("Settings"),
                    width: root.width - (LingmoUI.Units.gridUnit * 4),
                    height: root.height - (LingmoUI.Units.gridUnit * 4)
                })
            }
        ]
    }

    pageStack.initialPage: LingmoUI.Page {
        title: qsTr("Main Page")
    }
}
