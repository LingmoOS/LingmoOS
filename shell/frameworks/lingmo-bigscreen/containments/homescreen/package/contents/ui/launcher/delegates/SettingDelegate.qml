/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.private.nanoshell as NanoShell

BigScreen.IconDelegate {
    id: delegate
    icon.name: modelData.kcmIconName
    text: modelData.kcmName
    useIconColors: plasmoid.configuration.coloredTiles
    compactMode: plasmoid.configuration.expandingTiles

    onClicked: {
        BigScreen.NavigationSoundEffects.playClickedSound()
        NanoShell.StartupFeedback.open(
                            delegate.icon.name,
                            delegate.text,
                            delegate.LingmoUI.ScenePosition.x + delegate.width/2,
                            delegate.LingmoUI.ScenePosition.y + delegate.height/2,
                            Math.min(delegate.width, delegate.height), delegate.LingmoUI.Theme.backgroundColor);
        settingActions.launchSettings(modelData.kcmId)
        recentView.forceActiveFocus();
        recentView.currentIndex = 0;
    }
}
