// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.Page {
    id: page

    Connections {
        target: reportInterface
        function onDone(number) {
            pageStack.replace("qrc:/ui/SentPage.qml", {bugNumber: reportInterface.sentReport})
        }
        function onSendReportError(msg) {
            console.log("ERRROR" + msg)
            // TODO should going back be a thing?
            // pageStack.globalToolBar.showNavigationButtons = LingmoUI.ApplicationHeaderStyle.Auto
            inlineMessage.errorContext = msg
            page.state = "error"
        }
    }

    Connections {
        target: bugzilla
        function onLoginFinished() {
            console.log("login success")
            reportInterface.sendBugReport()
        }
        function onLoginError(msg) {
            console.log("login ERRROR " + msg)
            inlineMessage.errorContext = msg
            page.state = "error"
        }
    }

    actions: [
        LingmoUI.Action {
            visible: page.state === "error"
            icon.name: "document-save"
            text: i18nc("@action:button", "Save Report to File")
            tooltip: xi18nc("@info:tooltip", 'Use this button to save the crash information to a file for manual reporting.')
            onTriggered: DrKonqi.saveReport(reportInterface.generateReportFullText(ReportInterface.DrKonqiStamp.Include, ReportInterface.Backtrace.Complete))
        }
    ]

    ColumnLayout {
        anchors.fill: parent
        visible: page.state === "error"

        LingmoUI.InlineMessage {
            id: inlineMessage

            property string errorContext: ""

            Layout.fillWidth: true
            visible: true
            type: LingmoUI.MessageType.Error
            text: xi18nc("@info", "Failed to submit bug report: <message>%1</message>", errorContext)
            actions: [
                LingmoUI.Action {
                    icon.name: "document-send"
                    text: i18nc("@action retry submitting bug report", "Retry Submission")
                    onTriggered: {
                        inlineMessage.errorContext = ""
                        bugzilla.refreshToken()
                    }
                }
            ]
        }
    }

    LingmoUI.PlaceholderMessage {
        visible: page.state ===  ""
        anchors.centerIn: parent
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        text: i18nc("@info", "Submitting bug report...")

        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
    }

    Component.onCompleted: {
        // Disable navigation from here on out, the report can't be changed anymore!
        pageStack.globalToolBar.showNavigationButtons = LingmoUI.ApplicationHeaderStyle.NoNavigationButtons

        // Trigger relogin. If the user took a long time to prepare the login our
        // token might have gone invalid in the meantime. As a cheap way to prevent
        // this we'll simply refresh the token regardless. It's plenty cheap and
        // should reliably ensure that the token is current.
        // Disconnect everything first though, this function may get called a bunch
        // of times, so we don't want duplicated submissions.
        bugzilla.refreshToken();
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
