/*
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the QtDeclarative module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1
*/


import Qt 4.7

Rectangle {
    id: container

    property string text: "Button"

    signal clicked

    width: buttonLabel.width + 20; height: buttonLabel.height + 6
    smooth: true
    border { width: 1; color: Qt.darker(activePalette.button) }
    radius: 8
    color: activePalette.button

    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: {
                if (mouseArea.pressed)
                    return activePalette.dark
                else
                    return activePalette.light
            }
        }
        GradientStop { position: 1.0; color: activePalette.button }
    }

    MouseArea { id: mouseArea; anchors.fill: parent; onClicked: container.clicked() }

    Text {
        id: buttonLabel; text: container.text; anchors.centerIn: container; color: activePalette.buttonText
    }
}
