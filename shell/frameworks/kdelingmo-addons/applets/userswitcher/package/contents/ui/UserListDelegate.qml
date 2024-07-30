/*
 *  SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQml

import org.kde.config as KConfig
import org.kde.kcmutils as KCMUtils
import org.kde.lingmoui as LingmoUI
import org.kde.lingmouiaddons.components as LingmoUIComponents

ListDelegate {
    id: item

    property alias source: avatar.source

    iconItem: LingmoUIComponents.AvatarButton {
        id: avatar

        anchors.fill: parent

        name: item.text

        // don't block mouse hover from the underlying ListView highlight
        enabled: KConfig.KAuthorized.authorizeControlModule("kcm_users")

        onClicked: KCMUtils.KCMLauncher.openSystemSettings("kcm_users")
    }
}
