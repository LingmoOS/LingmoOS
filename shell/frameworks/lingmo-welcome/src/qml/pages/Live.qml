/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

GenericPage {
    id: root

    property bool installerAvailable: Config.liveInstaller.length !== 0

    heading: i18nc("@title:window %1 is the name of the user's distro", "Welcome to %1!", Controller.distroName())
    description: installerAvailable
                 ? xi18nc("@info:usagetip %1 is the name of the user's distro",
                          "Pressing the icon below will begin installing %1. Alternatively, you can close the window to explore the live environment or continue here to find out about KDE Lingmo.", Controller.distroName())
                 : xi18nc("@info:usagetip %1 is the name of the user's distro",
                          "You can close the window to explore the live environment or continue here to find out about KDE Lingmo.")

    topContent: [
        LingmoUI.UrlButton {
            id: link
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@action:button", "Visit home page")
            url: Controller.distroUrl()
            visible: url.length !== 0
        }
    ]

    LingmoUI.Icon {
        anchors.centerIn: parent
        width: Math.min(parent.height, LingmoUI.Units.gridUnit * 16)
        height: width

        layer.enabled: installerAvailable
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 1.0
            blurMax: 10
        }
        opacity: installerAvailable ? 0.1 : 1

        source: Controller.distroIcon()
        fallback: ""
    }

    ApplicationIcon {
        anchors.centerIn: parent
        application: ApplicationInfo {
            id: application
            desktopName: Config.liveInstaller
        }
        size: LingmoUI.Units.gridUnit * 10
        visible: root.installerAvailable
    }
}
