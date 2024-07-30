/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

import org.kde.plasma.welcome

GenericPage {
    id: root

    heading: i18nc("@info:window", "Activities")
    description: xi18nc("@info:usagetip", "Activities can be used to separate high-level projects or workflows so you can focus on one at a time. You can have an activity for “Home”, “School”, “Work”, and so on. Each Activity has access to all your files but has its own set of open apps and windows, recent documents, “Favorite” apps, and desktop widgets.")

    actions: [
        Kirigami.Action {
            icon.name: "preferences-desktop-activities"
            text: i18nc("@action:button", "Open Settings…")
            onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kcm_activities")
        }
    ]

    ColumnLayout {
        anchors.fill: parent

        spacing: root.padding

        Kirigami.AbstractCard {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MockDesktop {
                id: mockDesktop
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing

                backgroundAlignment: Qt.AlignLeft | Qt.AlignTop

                MockActivities {
                    anchors.top: parent.top
                    anchors.left: parent.left

                    width: Math.min(mockDesktop.width - root.padding, implicitWidth)

                    height: mockDesktop.height
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true

            text: xi18nc("@info:usagetip", "To get started, launch <interface>System Settings</interface> and search for “Activities”. On that page, you can create more Activities. You can then switch between them using the <shortcut>Meta+Tab</shortcut> keyboard shortcut.")
            wrapMode: Text.WordWrap
        }
    }
}
