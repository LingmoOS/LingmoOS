/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kquickcontrols 2.0
import org.kde.globalaccel 1.0

ApplicationWindow
{
    visible: true

    ListView {
        anchors.fill: parent
        model: ListModel {
            ListElement { text: "Hola1"; sequence: "Meta+X" }
            ListElement { text: "Hola2"; sequence: "Meta+Shift+X" }
            ListElement { text: "Hola3"; sequence: "Meta+Y" }
        }

        delegate: Rectangle {
            width: parent.width
            height: 100
            color: action.active ? "green" : "yellow"

            GlobalAction {
                id: action
                text: model.text
                onTriggered: console.log("triggered", text)
                objectName: "org.kde.globalaccel.test.globalacceltest."+model.text
                shortcut: sequenceItem.keySequence
            }

            KeySequenceItem
            {
                id: sequenceItem
                modifierlessAllowed: false
                keySequence: model.sequence
            }
        }
    }
}
