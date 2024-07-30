/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

LingmoUI.Page {
    title: qsTr("Page 1")

    property alias textField1: textField1
    property alias button1: button1

    actions: [
        LingmoUI.Action {
            text: "Sync"
            icon.name: "folder-sync"
            onTriggered: showPassiveNotification("Action clicked")
        }
    ]

    RowLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
        anchors.top: parent.top

        TextField {
            id: textField1
            placeholderText: qsTr("Text Field")
        }

        Button {
            id: button1
            text: qsTr("Press Me")
        }
    }
}
