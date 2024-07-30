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

    heading: i18nc("@info:window", "KRunner")
    description: xi18nc("@info:usagetip", "KRunner is Plasma’s exceptionally powerful and versatile search system. It powers the search functionality in the Application Launcher menu and the Overview screen, and it can be accessed as a standalone search bar using the <shortcut>Alt+Space</shortcut> keyboard shortcut.<nl/><nl/>In addition to finding your files and folders, KRunner can launch apps, search the web, convert between currencies, calculate math problems, and a lot more.")

    actions: [
        Kirigami.Action {
            icon.name: "krunner"
            text: i18nc("@action:button", "Open Settings…")
            onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kcm_plasmasearch")
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

                backgroundAlignment: Qt.AlignHCenter | Qt.AlignTop

                MockKRunner {
                    id: mockKRunner
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top

                    width: Math.min(mockDesktop.width - (root.padding * 2), implicitWidth)
                    height: Math.min(mockDesktop.height - root.padding, implicitHeight)
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true

            text: xi18nc("@info:usagetip", "To learn more, open the KRunner search bar using the <shortcut>Alt+Space</shortcut> keyboard shortcut and click on the question mark icon.")
            wrapMode: Text.WordWrap
        }
    }

    Timer {
        id: typingTimer

        property int index: 0 // The string being typed
        property int character: 0 // The character to type, -1 meaning clear
        readonly property var strings: [
                                        i18nc("@info:usagetip An example KRunner query, ensure keywords are localized as they are in the actual runner plugins. If needed, change 'Shanghai' to a city that on the other side of the world from likely speakers of the language.",
                                              "time Shanghai"),
                                        i18nc("@info:usagetip An example KRunner query, ensure keywords are localized as they are in the actual runner plugins.",
                                              "27 / 3"),
                                        i18nc("@info:usagetip An example KRunner query, ensure keywords are localized as they are in the actual runner plugins. If needed, change the currency conversion to a likely one for speakers of the language.",
                                              "200 EUR in USD"),
                                        i18nc("@info:usagetip An example KRunner query, ensure keywords are localized as they are in the actual runner plugins. If needed, change the conversion to a likely one for speakers of the language.",
                                              "25 miles in km"),
                                        i18nc("@info:usagetip An addendum to example KRunner queries, intentionally using '...' as these are the characters a person would type for ellipses",
                                              "...and much more!")
                                       ]

        // If we're clearing, the interval should be very short
        // It doesn't 1:1 match the behaviour of holding backspace, as
        // there is a delay before repeating but let's not go overboard...
        interval: character == -1 ? Kirigami.Units.veryShortDuration : Kirigami.Units.shortDuration

        running: true
        repeat: true

        onTriggered: {
            if (character != -1) {
                // Typing
                mockKRunner.searchText += strings[index].charAt(character++);

                // End of string
                if (character >= strings[index].length) {
                    character = -1;
                    index = (index + 1) % strings.length;

                    typingTimer.stop();
                    pauseTimer.start();
                }
            } else {
                // Clearing
                mockKRunner.searchText = mockKRunner.searchText.substring(0, mockKRunner.searchText.length - 1);

                // Empty box
                if (mockKRunner.searchText.length == 0) {
                    character = 0;
                }
            }
        }
    }

    Timer {
        id: pauseTimer

        interval: Kirigami.Units.humanMoment
        onTriggered: typingTimer.start()
    }
}
