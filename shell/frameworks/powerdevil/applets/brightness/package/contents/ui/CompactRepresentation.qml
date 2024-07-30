/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>
    SPDX-FileCopyrightText: 2013 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.workspace.components as WorkspaceComponents
import org.kde.lingmoui as LingmoUI

MouseArea {
    id: root

    readonly property bool isConstrained: Plasmoid.formFactor === LingmoCore.Types.Vertical || Plasmoid.formFactor === LingmoCore.Types.Horizontal
    property real brightnessError: 0
    property bool isBrightnessAvailable: false

    activeFocusOnTab: true
    hoverEnabled: true

    Accessible.name: Plasmoid.title
    Accessible.description: `${toolTipMainText}; ${toolTipSubText}`
    Accessible.role: Accessible.Button

    LingmoUI.Icon {
        anchors.fill: parent
        source: Plasmoid.icon
        active: root.containsMouse
    }

}
