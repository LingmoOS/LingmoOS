/*
    SPDX-FileCopyrightText: 2022 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.2

import org.kde.lingmo.components 3.0 as LingmoComponents3

import org.kde.ocean.components

SessionManagementScreen {
    focus: true
    LingmoComponents3.Button {
        id: loginButton
        focus: true
        text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Unlock")
        icon.name: "unlock"
        onClicked: Qt.quit();
        Keys.onEnterPressed: clicked()
        Keys.onReturnPressed: clicked()
    }

    Component.onCompleted: {
        forceActiveFocus();
        Qt.callLater(tryToSwitchUser, false)
    }
}
