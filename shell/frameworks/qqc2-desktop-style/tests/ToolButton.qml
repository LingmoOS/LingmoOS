/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    GridLayout {
        anchors.fill: parent
        anchors.margins: 10
        rows: 7
        flow: GridLayout.TopToBottom

        Label {
            text: "Flat"
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Default"
            flat: true
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Icon Only"
            display: ToolButton.IconOnly
            flat: true
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Only"
            display: ToolButton.TextOnly
            flat: true
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Beside Icon"
            display: ToolButton.TextBesideIcon
            flat: true
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Below Icon"
            display: ToolButton.TextUnderIcon
            flat: true
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button highlighted"
            flat: true
            highlighted: true
        }

        Label {
            text: "Non-Flat"
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Default"
            flat: false
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Icon Only"
            display: ToolButton.IconOnly
            flat: false
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Only"
            display: ToolButton.TextOnly
            flat: false
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Beside Icon"
            display: ToolButton.TextBesideIcon
            flat: false
        }

        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button Text Below Icon"
            display: ToolButton.TextUnderIcon
            flat: false
        }
        ToolButton {
            icon.name: "documentinfo"
            text: "Tool Button highlighted"
            flat: false
            highlighted: true
        }

        ToolButton {
            text: "Without setting display"
        }

        ToolButton {
            text: "With Menu decoration"

            Accessible.role: Accessible.ButtonMenu
        }
    }
}
