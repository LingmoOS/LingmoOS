// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.ScrollablePage {
    id: page

    title: i18nc("@title:window", "Welcome to the Reporting Assistant")

    ColumnLayout {
        QQC2.Label {
            Layout.fillWidth: true
            text: xi18nc("@info/rich", `This assistant will analyze the crash information and guide you through the bug reporting process.`)
            wrapMode: Text.Wrap
        }
        RowLayout {
            LingmoUI.Icon {
                implicitWidth: LingmoUI.Units.iconSizes.large
                implicitHeight: implicitWidth
                source: "dialog-warning"
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: xi18nc("@info/rich note before starting the bug reporting process",
`<para><note>Since communication between you and the developers is required for effective debugging,
to continue reporting this bug it is <emphasis strong='true'>required for you to agree that developers may contact you</emphasis>.
</note></para><para>Feel free to close this dialog if you do not accept this.</para>`)
                wrapMode: Text.Wrap
            }
        }
        // FIXME: maybe even disable i18n for the rest of the dialog seeing as one must be proficient enough in english
    }

    footer: FooterActionBar {
        actions: [
            LingmoUI.Action {
                icon.name: "document-sign"
                text: i18nc("@action:button", "I Agree to be Contacted")
                onTriggered: {
                    visible = false
                    pageStack.push("qrc:/ui/ContextPage.qml")
                }
                visible: true
            }
        ]
    }
}
