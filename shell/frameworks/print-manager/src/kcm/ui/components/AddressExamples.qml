/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: root
    signal selected(string address)

    property var examples: []

    Component.onCompleted: {
        if (background) {
            background.visible = true
        }
    }

    contentItem: ListView {
        clip: true
        model: examples

        headerPositioning: ListView.OverlayHeader
        header: Kirigami.InlineViewHeader {
            text: i18nc("@info:usagetip", "Example Addresses")
            implicitWidth: ListView.view.width
        }

        delegate: QQC2.ItemDelegate {
            implicitWidth: ListView.view.width
            text: modelData
            onClicked: root.selected(modelData)
        }
    }
}
