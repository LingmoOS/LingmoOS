// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

import org.kde.drkonqi as DrKonqi

RowLayout {
    spacing: LingmoUI.Units.smallSpacing

    QQC2.CheckBox {
        checked: DrKonqi.Settings.downloadSymbols
        text: i18nc("@label", "Automatically download debug symbols to enhance crash reports")
        onToggled: {
            DrKonqi.Settings.downloadSymbols = checked
            DrKonqi.Settings.save()
        }
    }

    LingmoUI.ContextualHelpButton {
        toolTipText: i18nc("@info:tooltip",
`Crash reports can be of greater value if additional debugging resources are downloaded from your distributor first.
This causes downloads of unknown size when a crash occurs. This is skipped automatically when using a metered internet connection.`)
    }
}
