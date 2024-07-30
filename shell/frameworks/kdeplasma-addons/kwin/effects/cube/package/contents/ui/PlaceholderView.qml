/*
    SPDX-FileCopyrightText: 2024 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kwin as KWinComponents
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras

Rectangle {
    color: Kirigami.Theme.backgroundColor
    focus: true

    TapHandler {
        onTapped: effect.deactivate();
    }

    PlasmaExtras.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - Kirigami.Units.gridUnit * 2
        iconName: "virtual-desktops"
        text: i18ndcp("kwin_effect_cube",
                      "@info:placeholder",
                      "At least 3 virtual desktops are required to display the Cube, but only %1 is present",
                      "At least 3 virtual desktops are required to display the Cube, but only %1 are present",
                      KWinComponents.Workspace.desktops.length)
        helpfulAction: Action {
            text: i18ndc("kwin_effect_cube", "@action:button", "Add Virtual Desktop")
            icon.name: "list-add-symbolic"
            onTriggered: KWinComponents.Workspace.createDesktop(KWinComponents.Workspace.desktops.length, "")
        }
    }

    Keys.onEscapePressed: effect.deactivate();
}
