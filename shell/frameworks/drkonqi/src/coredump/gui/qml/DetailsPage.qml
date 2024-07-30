// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI
import org.kde.syntaxhighlighting 1.0

import org.kde.drkonqi.coredump.gui 1.0 as DrKonqi

LingmoUI.ScrollablePage {
    id: page

    property alias patient: detailsLoader.patient
    property string text
    property string errorText

    title: i18nc("@title", "Details")
    horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View

    actions: [
        LingmoUI.Action {
            enabled: state === ""
            icon.name: "edit-copy"
            text: i18nc("@action", "Copy to Clipboard")
            onTriggered: {
                contentLoader.item.selectAll()
                contentLoader.item.copy()
            }
        },
        LingmoUI.Action {
            enabled: patient.canDebug
            icon.name: "debug-run"
            text: i18nc("@action", "Run Interactive Debugger")
            onTriggered: patient.debug()
        }
    ]

    Component {
        id: loadingComponent
        QQC2.BusyIndicator {
            id: indicator
            visible: false
            running: true

            // only show the indicator after a brief timeout otherwise we can have a situtation where loading takes a couple
            // milliseconds during which time the indicator flashes up for no good reason
            Timer {
                running: true
                repeat: false
                interval: 500
                onTriggered: indicator.visible = true
            }
        }
    }

    Component {
        id: errorComponent

        LingmoUI.PlaceholderMessage {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            text: page.errorText
            icon.name: "data-warning"
        }
    }

    Component {
        id: dataComponent

        QQC2.TextArea {
            id: textfield
            Layout.fillWidth: true
            Layout.fillHeight: true
            readOnly: true
            wrapMode: TextEdit.NoWrap
            textFormat: TextEdit.PlainText
            background.visible: false
            font.family: "monospace"
            text: page.text
            selectByMouse: LingmoUI.Settings.isMobile ? false : true

            SyntaxHighlighter {
                textEdit: textfield
                definition: "GDB Backtrace"
            }
        }
    }

    Loader {
        id: contentLoader

        DrKonqi.DetailsLoader {
            id: detailsLoader
            onDetails: details => text = details
            onError: error => {
                console.log("error" + error)
                errorText = error
            }
        }
    }

    states: [
        State {
            name: "error"
            when: errorText !== ""
            PropertyChanges { target: contentLoader; sourceComponent: errorComponent }
        },
        State {
            name: "loading"
            when: text === ""
            PropertyChanges { target: contentLoader; sourceComponent: loadingComponent }
        },
        State {
            name: "" // default state
            PropertyChanges { target: contentLoader; sourceComponent: dataComponent }
        }
    ]
}
