/*
    SPDX-FileCopyrightText: 2019 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2020 Chris Holland <zrenfire@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow
{
    visible: true

    ColumnLayout {
        anchors.fill: parent
        ComboBox {
            Layout.fillWidth: true
            textRole: "key"
            model: ListModel {
                id: comboModel
                ListElement { key: "First"; value: 123 }
                ListElement { key: "Second"; value: 456 }
                ListElement { key: "Third"; value: 789 }
            }
        }

        ComboBox {
            Layout.fillWidth: true
            textRole: "key"
            model: comboModel
            editable: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: comboModel
            delegate: Label { text: key }
        }
    }
}
