/*
    SPDX-FileCopyrightText: 2023 Thenujan Sandramohan <sthenujan2002@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.kcm.autostart

KCM.SimpleKCM {
    id: entryPage

    property Unit unit

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    onBackRequested: kcm.pop()

    actions: QQC2.Action {
        icon.name: entryPage.unit.activeState === "active" ? "media-playback-stop-symbolic" : "media-playback-start-symbolic"
        text: entryPage.unit.activeState === "active"
            ? i18nc("@label Stop the Systemd unit for a running process", "Stop")
            : i18nc("@label Start the Systemd unit for a currently inactive process", "Start")

        onTriggered: {
            if (entryPage.unit.activeState === "active") {
                entryPage.unit.stop();
            } else {
                entryPage.unit.start();
            }
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: ColumnLayout {
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.InlineMessage {
            id: errorMessage

            Layout.fillWidth: true
            position: LingmoUI.InlineMessage.Position.Header
            showCloseButton: true

            property bool journalError: false

            Connections {
                target: entryPage.unit
                function onJournalError(message) {
                    errorMessage.type = LingmoUI.MessageType.Error;
                    errorMessage.visible = true;
                    errorMessage.text = message;
                    errorMessage.journalError = true;
                }
            }
        }

        LingmoUI.FormLayout {
            // Equal to the margins removed by disabling header padding
            Layout.margins: LingmoUI.Units.mediumSpacing

            QQC2.Label {
                LingmoUI.FormData.label: i18nc("@label The name of a Systemd unit for an app or script that will autostart", "Name:")
                text: entryPage.unit.description
                textFormat: Text.PlainText
            }

            QQC2.Label {
                LingmoUI.FormData.label: i18nc("@label The current status (e.g. active or inactive) of a Systemd unit for an app or script that will autostart", "Status:")
                text: entryPage.unit.activeStateValue
                textFormat: Text.PlainText
            }

            QQC2.Label {
                LingmoUI.FormData.label: i18nc("@label A date and time follows this text, making a sentence like 'Last activated on: August 7th 11 PM 2023'", "Last activated on:")
                text: entryPage.unit.timeActivated
                textFormat: Text.PlainText
            }
        }
    }

    QQC2.Label {
        id: sheetLabel
        text: entryPage.unit.logs
                .replace(/LingmoUI.Theme.neutralTextColor/g, LingmoUI.Theme.neutralTextColor)
                .replace(/LingmoUI.Theme.negativeTextColor/g, LingmoUI.Theme.negativeTextColor)
                .replace(/LingmoUI.Theme.positiveTextColor/g, LingmoUI.Theme.positiveTextColor)
        padding: LingmoUI.Units.largeSpacing
        wrapMode: Text.WordWrap
        anchors.fill: parent

        background: Rectangle {
            color: LingmoUI.Theme.backgroundColor
            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            LingmoUI.Theme.inherit: false
        }
        textFormat: Text.StyledText
    }
    LingmoUI.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)

        visible: entryPage.unit.logs == ''

        icon.name: "data-error-symbolic"
        text: i18n("Unable to load logs. Try refreshing.")

        helpfulAction: LingmoUI.Action {
            icon.name: "view-refresh-symbolic"
            text: i18nc("@action:button Refresh entry logs when it failed to load", "Refresh")
            onTriggered: entryPage.unit.reloadLogs()
        }

    }

    Timer {
        interval: LingmoUI.Units.humanMoment
        running: true
        repeat: true
        onTriggered: {
            // Reloading logs when journal error occurs will cause repeated errorMessage. So we try to avoid it
            if (!errorMessage.journalError) {
                entryPage.unit.reloadLogs()
            }
        }
    }

}
