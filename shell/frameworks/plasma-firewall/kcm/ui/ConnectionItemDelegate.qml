// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12 as QQC2

import org.kde.kirigami 2.4 as Kirigami


QQC2.ItemDelegate {
    id: root

    hoverEnabled: true

    signal filterConnection(var protocol, var localAddress, var foreignAddres, var status)

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.preferredWidth: 60
            text: model.protocol
        }
        QQC2.Label {
            Layout.preferredWidth: 160
            text: model.localAddress
        }
        QQC2.Label {
            Layout.preferredWidth: 160
            text: model.foreignAddress
        }
        QQC2.Label {
            Layout.preferredWidth: 100
            text: model.status
        }
        QQC2.Label {
            Layout.preferredWidth: 40
            text: model.pid
        }
        QQC2.Label {
            Layout.preferredWidth: 120
            text: model.program !== "" ? model.program : ""
        }
        Item {
            visible: !root.hovered
            implicitHeight: iconButton.height
            implicitWidth: iconButton.width
        }
        QQC2.ToolButton {
            id: iconButton
            visible: root.hovered
            icon.name: "view-filter"
            onClicked: root.filterConnection(model.protocol, model.localAddress,
                                                model.foreignAddress, model.status)
        }
    }
}
