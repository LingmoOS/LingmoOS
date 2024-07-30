/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

RowLayout {
    spacing: LingmoUI.Units.smallSpacing

    QQC2.Switch {
        Layout.alignment: Qt.AlignLeft

        text: i18nc("@option:check", "Show this page after Lingmo is updated")
        checked: Config.showUpdatePage
        onToggled: { Config.showUpdatePage = checked; Config.save() }
    }

    QQC2.Button {
        id: okButton
        Layout.alignment: Qt.AlignRight

        action: LingmoUI.Action {
            text: i18nc("@action:button", "&OK")
            icon.name: "dialog-ok-apply-symbolic"
            shortcut: Qt.application.layoutDirection === Qt.LeftToRight ? "Right" : "Left"

            onTriggered: Qt.quit()
        }
    }
}
