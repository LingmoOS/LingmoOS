/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

GenericPage {
    heading: i18nc("@info:window", "System Settings")
    description: xi18nc("@info:usagetip", "The System Settings app lets you extensively customize the system to suit your tastes and preferences. Default apps, wallpapers, visual styles and icons, virtual desktops, power management, 3rd-party add-ons — you’ll find it all there!")

    ApplicationIcon {
        anchors.centerIn: parent

        application: ApplicationInfo {
            id: application
            desktopName: "systemsettings"
        }

        size: Kirigami.Units.gridUnit * 10
    }
}
