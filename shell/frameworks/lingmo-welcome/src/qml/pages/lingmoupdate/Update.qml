/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

ColumnLayout {
    id: root

    readonly property string description: xi18nc("@info:usagetip", "KDE contributors have spent the last four months hard at work on this release. We hope you enjoy using Lingmo as much as we enjoyed making it!")
    readonly property int iconSize: LingmoUI.Units.iconSizes.enormous

    readonly property list<QtObject> topContent: [
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@action:button", "<b>Find out what's new</b>")
            url: Controller.releaseUrl
        },
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@action:button", "Help work on the next release")
            url: "https://community.kde.org/Get_Involved?source=lingmo-welcome"
        }
    ]

    spacing: LingmoUI.Units.gridUnit

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillHeight: true
        Layout.maximumHeight: Number.POSITIVE_INFINITY

        spacing: LingmoUI.Units.gridUnit

        LingmoUI.Icon {
            Layout.preferredWidth: root.iconSize
            Layout.preferredHeight: root.iconSize
            source: "start-here-kde-lingmo"
        }

        QQC2.Label {
            text: "="
            font.pointSize: 72
        }

        LingmoUI.Icon {
            Layout.preferredWidth: root.iconSize
            Layout.preferredHeight: root.iconSize
            source: "face-in-love"
        }
    }

    LingmoUI.AbstractCard {
        Layout.fillWidth: true

        contentItem: ColumnLayout {
            spacing: LingmoUI.Units.smallSpacing

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: xi18nc("@info:usagetip", "The KDE community relies on donations of expertise and funds, and is supported by KDE e.V.—a German nonprofit. Donations to KDE e.V. support the wider KDE community, and you can make a difference by donating today.")
            }

            LingmoUI.UrlButton {
                text: i18nc("@action:button", "Make a donation")
                url: "https://kde.org/community/donations?source=lingmo-welcome"
            }
        }
    }
}
