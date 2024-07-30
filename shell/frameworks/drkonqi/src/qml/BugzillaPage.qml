// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.Page {
    id: page
    // This item is intentionally not TITLED. The page should usually not show
    // up when when it shows up it should focus on the bare essentials!

    // FIXME currently this page does always show up because we don't preload it so we only contact bugzilla when the page is due for appearance

    Connections {
        target: bugzilla
        function onBugzillaVersionFound() {
            pageStack.pop()
            pageStack.push("qrc:/ui/LoginPage.qml")
        }

        function onBugzillaVersionError(error) {
            console.log("errro " + error)
            inlineMessage.errorContext = error
            page.state = "error"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        visible: page.state === "error"

        LingmoUI.InlineMessage {
            id: inlineMessage

            property string errorContext

            Layout.fillWidth: true
            type: LingmoUI.MessageType.Error
            text: xi18nc("@info", "Failed to contact bugs.kde.org: <message>%1</message>", errorContext)
            visible: true
            actions: [
                LingmoUI.Action {
                    icon.name: "cloudstatus"
                    text: i18nc("@action", "Retry")
                    onTriggered: {
                        page.state = ""
                        bugzilla.lookupVersion()
                    }
                }
            ]
        }
    }

    LingmoUI.PlaceholderMessage {
        visible: page.state === ""
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        text: i18nc("@info", "Trying to contact bugs.kde.org...")

        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
    }

    Component.onCompleted: {
        bugzilla.lookupVersion()
    }

    states: [
        State {
            name: "error"
        },
        State {
            name: "" // default state
        }
    ]
}
