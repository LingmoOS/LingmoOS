/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root

    required property bool isDistroPage
    required property string error

    title: i18nc("@info:window", "Error loading page")

    actions: [
        Kirigami.Action {
            icon.name: "tools-report-bug-symbolic"
            text: i18nc("@action:button", "Report Bug…")
            onTriggered: Qt.openUrlExternally(isDistroPage ? Controller.distroBugReportUrl() : "https://bugs.kde.org/enter_bug.cgi?product=Welcome%20Center")
        },
        Kirigami.Action {
            icon.name: "edit-copy-symbolic"
            text: i18nc("@action:button", "Copy Details")
            onTriggered: Controller.copyToClipboard(error)
        }
    ]

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent

        width: parent.width

        icon.name: "tools-report-bug"
        text: {
            if (isDistroPage) {
                return xi18nc("@info:usagetip", "This page provided by your distribution could not be loaded");
            } else {
                return xi18nc("@info:usagetip", "This page provided by Welcome Center could not be loaded");
            }
        }
        explanation: root.error
    }
}
