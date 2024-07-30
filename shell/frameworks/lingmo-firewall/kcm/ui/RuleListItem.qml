// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12 as QQC2

QQC2.ItemDelegate {
    id: itemRoot

    property bool dropAreasVisible: false

    signal edit(int index)
    signal remove(int index)

    height: 42

    onClicked: edit(index)

    contentItem: RowLayout {
        QQC2.Label {
            Layout.fillHeight: true
            Layout.leftMargin: 4
            text: model.action
        }
        QQC2.Label {
            text: model.from
        }
        QQC2.Label {
            text: model.to
        }
        QQC2.Label {
            text: model.logging
        }
        Item {
            visible: !eraseButton.visible
            width: eraseButton.width
            height: eraseButton.height
        }
        QQC2.ToolButton {
            id: eraseButton
            visible: itemRoot.hovered

            icon.name: "user-trash"
            onClicked: itemRoot.remove(index)
        }
        Item {
            Layout.fillWidth: true
        }
    }
}
