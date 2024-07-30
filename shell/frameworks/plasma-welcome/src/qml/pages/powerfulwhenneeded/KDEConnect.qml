/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

import org.kde.plasma.welcome

GenericPage {
    heading: i18nc("@info:window", "KDE Connect")

    // Don't change the weird indentation; it's intentional to make this
    // long string nicer for translators
    description: xi18nc("@info:usagetip", "KDE Connect lets you integrate your phone with your computer in various ways:\
<nl/>\
<list><item>See notifications from your phone on your computer</item>\
<item>Reply to text messages from your phone on your computer</item>\
<item>Sync your clipboard contents between your computer and your phone</item>\
<item>Make a noise on your phone when it’s been misplaced</item>\
<item>Copy pictures, videos, and other files from your phone to your computer, and vice versa</item>\
<item>…And much more!</item></list>\
<nl/>To get started, launch <interface>System Settings</interface> and search for “KDE Connect”. On that page, you can pair your phone.")

    // TODO: KDE Connect might not be installed:
    // We should show an InlineMessage and hide the action.

    actions: [
        Kirigami.Action {
            icon.name: "kdeconnect"
            text: i18nc("@action:button", "Open Settings…")
            onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kcm_kdeconnect")
        }
    ]
}
