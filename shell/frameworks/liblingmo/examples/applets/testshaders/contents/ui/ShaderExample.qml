/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

// VertexPage

LingmoComponents.Page {
    id: examplesPage

    //property string shader
    property string pageName: "Shader Examples"
    property string pageDescription: "Shader Examples"
    property string icon: "weather-clear"

    anchors {
        fill: parent
        margins: _s
    }

    LingmoUI.Heading {
        id: heading
        level: 1
        anchors {
            top: parent.top;
            left: parent.left
            right: parent.right
        }
        text: pageName
    }
    LingmoComponents.Label {
        anchors {
            top: heading.bottom;
            left: parent.left;
            right: parent.right;
        }
        text: pageDescription
    }
}
