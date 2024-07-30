// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.ScrollablePage {
    id: page

    title: i18nc("@title", "Preview the Report")

    ColumnLayout {
        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: xi18nc("@label/rich", "<para>This is a preview of the report's contents which will be sent.</para><para>If you want to modify it go to the previous pages.</para>")
        }

        // FIXME height is off maybe put the scrollview on the area instead of scrolling the page?
        QQC2.TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: contentHeight
            readOnly: true
            text: reportInterface.generateReportFullText(ReportInterface.DrKonqiStamp.Include, ReportInterface.Backtrace.Complete)
            wrapMode: TextEdit.Wrap

        }
    }

    footer: FooterActionBar {
        actions: [
            LingmoUI.Action {
                icon.name: "submit"
                text: i18nc("@action:button", "Submit")
                onTriggered: pageStack.push("qrc:/ui/SendingPage.qml")
            }
        ]
    }
}
