/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.components as LingmoComponents
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.lingmoui as LingmoUI

// ButtonsPage

LingmoComponents.Page {
    id: plasmoidPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        spacing: _s/2
        anchors.fill: parent
        LingmoUI.Heading {
            level: 1
            width: parent.width
            text: "Buttons"
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.Button {
                text: "Button"
                iconSource: "call-start"
            }
            LingmoComponents.ToolButton {
                text: "ToolButton"
                iconSource: "call-stop"
            }
        }
        Row {
            height: _h
            spacing: _s
            LingmoComponents.RadioButton {
                id: radio
                text: "RadioButton"
                //iconSource: "call-stop"
                onCheckedChanged: if (checked) tfield.forceActiveFocus()
            }
            LingmoComponents.TextField {
                id: tfield
                enabled: radio.checked
                text: "input here"
                clearButtonShown: true
            }
        }
//         LingmoComponents.TextArea {
//             width: parent.width
//             height: _h*2
//             wrapMode: TextEdit.Wrap
//         }
    }
}

