/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@ooutlook.com>
 *  SPDX-FileCopyrightText: 2024 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

GenericPage {
    id: root

    heading: i18nc("@title:window", "Enjoy It!")
    description: xi18nc("@info:usagetip", "We hope you love Lingmo as much as we loved making it for you! Now it’s time to jump right in. Explore its features, install your favorite apps and games, and get busy doing what makes you you!")

    topContent: [
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@label:URL", "Learn more about the KDE community")
            url: "https://community.kde.org/Welcome_to_KDE?source=lingmo-welcome"
        },
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@label:URL", "Get help at KDE’s discussion forums")
            url: "https://discuss.kde.org/?source=lingmo-welcome" // TODO: Discuss probably doesn't consume analytics or atleast in this way?
        }
    ]

    ColumnLayout {
        anchors.fill: parent

        spacing: root.padding

        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: LingmoUI.Units.gridUnit * 25

            fillMode: Image.PreserveAspectFit
            mipmap: true
            source: "konqi-default.png"
        }

        LingmoUI.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: LingmoUI.Units.smallSpacing

                QQC2.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: xi18nc("@info:usagetip", "If you find Lingmo to be useful, consider getting involved or donating. KDE is an international volunteer community, not a big company; your contributions make a real difference!")
                }

                LingmoUI.UrlButton {
                    text: i18nc("@action:button", "Make a donation")
                    url: "https://kde.org/community/donations?source=lingmo-welcome"
                }

                LingmoUI.UrlButton {
                    text: i18nc("@action:button", "Get involved")
                    url: "https://community.kde.org/Get_Involved?source=lingmo-welcome"
                }
            }
        }
    }

    actions: [
        LingmoUI.Action {
            icon.name: "favorite-symbolic"
            text: i18nc("@action:button", "Supporting Members")
            onTriggered: pageStack.layers.push(supporters)
        }
    ]

    Component {
        id: supporters

        Supporters {}
    }
}
