/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.plasma.workspace.dialogs as PWD

PWD.SystemDialog {
    id: root

    property alias description: desc.text
    property alias allowRestore: allowRestoreItem.checked
    property alias persistenceRequested: allowRestoreItem.visible

    iconName: "krfb"

    ColumnLayout {
        QQC2.Label {
            id: desc
            textFormat: Text.MarkdownText
            Layout.fillHeight: true
        }
        QQC2.CheckBox {
            id: allowRestoreItem
            checked: true
            text: i18n("Allow restoring on future sessions")
        }
    }

    standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel

    Component.onCompleted: {
        dialogButtonBox.standardButton(QQC2.DialogButtonBox.Ok).text = i18n("Share")
    }
}
