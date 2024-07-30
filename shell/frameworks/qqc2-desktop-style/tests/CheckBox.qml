/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2021 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
    width: 300
    height: layout.implicitHeight

    ColumnLayout {
        id: layout

        width: parent.width

        CheckBox {
        }

        CheckBox {
            checked: true
            enabled: false
        }

        CheckBox {
            text: "text"
        }

        CheckBox {
            icon.name: "checkmark"
        }

        CheckBox {
            text: "text plus icon"
            icon.name: "checkmark"
        }

        CheckBox {
            text: "focused"
            focus: true
        }

        CheckBox {
            text: "checked"
            checkState: Qt.Checked
        }

        CheckBox {
            text: "partially checked"
            checkState: Qt.PartiallyChecked
            tristate: true
        }

        CheckBox {
            text: "disabled"
            enabled: false
        }

        CheckBox {
            text: "disabled and checked"
            enabled: false
            checkState: Qt.Checked
        }

        CheckBox {
            text: "disabled and icon"
            enabled: false
            icon.name: "checkmark"
        }

        CheckBox {
            Layout.fillWidth: true
            text: "This is a very long piece of text that really should be rewritten to be shorter, but sometimes life just isn't that simple."
        }
    }
}

