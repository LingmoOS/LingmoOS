/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

import org.kde.bluezqt as BluezQt

ColumnLayout {
    id: root

    property var device: undefined

    anchors.bottomMargin: Kirigami.Units.smallSpacing

    Kirigami.Heading {
        Layout.fillWidth: true
        text: i18nd("purpose6_bluetooth", "Choose a device to send to:")
        visible: list.count !== 0
        level: 1
        wrapMode: Text.Wrap
    }

    ScrollView {
        id: scroll

        Layout.fillWidth: true
        Layout.fillHeight: true

        // Make sure we have space to show the placeholdermessage when no explicit size is set
        contentHeight: list.count !== 0 ? implicitContentHeight : Kirigami.Units.gridUnit * 3

        Component.onCompleted: scroll.background.visible = true

        ListView {
            id: list

            // Don't select anything by default; make the user choose explicitly
            currentIndex: -1

            clip: true
            model: BluezQt.DevicesModel { }

            delegate: ItemDelegate {
                width: ListView.view.width
                text: Name
                icon.name: Icon
                onClicked: root.device = Ubi
                checked: root.device === Ubi
                highlighted: root.device === Ubi
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: list.count === 0
                text: i18nd("purpose6_bluetooth", "No devices found")
            }
        }
    }
}
