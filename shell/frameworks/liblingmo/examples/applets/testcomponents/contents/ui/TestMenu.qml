/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as LingmoComponents
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

// PlasmoidPage

LingmoComponents.Menu {
    id: testMenu

    LingmoComponents.MenuItem {
        text: "Red Snapper"
        icon: "dragonplayer"
        onClicked: print(" Clicked on : " + text)
    }

    LingmoComponents.MenuItem {
        text: "Eel"
        icon: "kthesaurus"
        onClicked: print(" Clicked on : " + text)
    }

    LingmoComponents.MenuItem {
        text: "White Tip Reef Shark"
        icon: "kmag"
        onClicked: print(" Clicked on : " + text)
    }

    Component.onCompleted:{
        print("TestMenu.qml served .. opening");

    }
}

