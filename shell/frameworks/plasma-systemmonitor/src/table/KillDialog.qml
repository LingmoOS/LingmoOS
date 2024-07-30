/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: dialog

    required property string killButtonText
    required property string killButtonIcon
    required property string questionText

    property var items: []
    property bool doNotAskAgain: false
    property alias delegate: list.delegate

    preferredWidth: Kirigami.Units.gridUnit * 25

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            text: dialog.killButtonText
            icon.name: dialog.killButtonIcon
            onTriggered: dialog.accept()
        }
    ]

    contentItem: ScrollView {
        implicitWidth: Kirigami.Units.gridUnit * 25

        leftPadding: Kirigami.Units.largeSpacing
        rightPadding: Kirigami.Units.largeSpacing
        topPadding: Kirigami.Units.largeSpacing
        bottomPadding: Kirigami.Units.largeSpacing

        ListView {
            id: list

            header: Label {
                padding: Kirigami.Units.largeSpacing
                width: list.width - list.leftMargin - list.rightMargin
                text: dialog.questionText
                wrapMode: Text.Wrap
            }

            model: dialog.items
            currentIndex: -1
            clip: true

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            delegate: ItemDelegate {
                width: ListView.view.width
                text: modelData

                // We don't want visual interactivity for the background
                highlighted: false
                hoverEnabled: false
                down: false
            }
        }
    }

    footerLeadingComponent: CheckBox {
        leftPadding: Kirigami.Units.largeSpacing
        text: i18ndc("plasma-systemmonitor", "@option:check", "Do not ask again");
        onToggled: dialog.doNotAskAgain = checked
    }
}
