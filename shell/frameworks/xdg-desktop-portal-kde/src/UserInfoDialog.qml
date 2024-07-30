/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.workspace.dialogs as PWD

PWD.SystemDialog {
    id: root

    property alias reason: reasonLabel.text
    property alias userName: idText.text
    property alias realName: nameText.text

    ColumnLayout {
        Kirigami.Heading {
            id: nameText
            Layout.fillWidth: true
        }
        Kirigami.Heading {
            id: idText
            Layout.fillWidth: true
            level: 3
        }

        QQC2.Label {
            id: reasonLabel
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
    }

    standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel

    Component.onCompleted: {
        dialogButtonBox.standardButton(QQC2.DialogButtonBox.Ok).text = i18n("Share")
    }
}
