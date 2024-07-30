/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.private.nanoshell as NanoShell
import org.kde.lingmo.plasmoid 2.0

BigScreen.IconDelegate {
    id: delegate
    readonly property var appStorageIdRole: modelData.ApplicationStorageIdRole

    icon.name: modelData ? modelData.ApplicationIconRole : ""
    text: modelData ? modelData.ApplicationNameRole : ""
    useIconColors: plasmoid.configuration.coloredTiles
    compactMode: plasmoid.configuration.expandingTiles

    onClicked: {
        BigScreen.NavigationSoundEffects.playClickedSound()
        NanoShell.StartupFeedback.open(
                            delegate.icon.name.length > 0 ? delegate.icon.name : model.decoration,
                            delegate.text,
                            delegate.LingmoUI.ScenePosition.x + delegate.width/2,
                            delegate.LingmoUI.ScenePosition.y + delegate.height/2,
                            Math.min(delegate.width, delegate.height), delegate.LingmoUI.Theme.backgroundColor);
        plasmoid.applicationListModel.runApplication(modelData.ApplicationStorageIdRole)
        recentView.forceActiveFocus();
        recentView.currentIndex = 0;
    }
}
