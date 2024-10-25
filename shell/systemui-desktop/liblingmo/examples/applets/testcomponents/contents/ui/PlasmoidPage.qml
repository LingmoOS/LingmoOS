/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.lingmoui as LingmoUI

// PlasmoidPage

LingmoComponents.Page {
    id: plasmoidPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        anchors.centerIn: parent
        spacing: _s
        LingmoUI.Heading {
            level: 2
            text: "I'm an applet"
        }

        LingmoComponents.ButtonColumn {
            LingmoComponents.RadioButton {
                text: "No background"
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 0;
                }
            }
            LingmoComponents.RadioButton {
                text: "Default background"
                checked: true
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 1;
                }
            }
            LingmoComponents.RadioButton {
                text: "Translucent background"
                onClicked: {
                    if (checked) Plasmoid.backgroundHints = 2;
                }
            }
        }

        LingmoComponents.Button {
            height: LingmoUI.Units.iconSizes.desktop
            text: "Busy"
            checked: Plasmoid.busy
            onClicked: {
                Plasmoid.busy = !Plasmoid.busy
            }
        }

        LingmoComponents.Button {
            id: ctxButton
            height: LingmoUI.Units.iconSizes.desktop
            text: "Context Menu"
            Loader {
                id: menuLoader
            }
            onClicked: {
                if (menuLoader.source == "") {
                   menuLoader.source = "TestMenu.qml"
                } else {
                    //menuLoader.source = ""
                }
                menuLoader.item.open(0, height);
            }
        }
    }
}

