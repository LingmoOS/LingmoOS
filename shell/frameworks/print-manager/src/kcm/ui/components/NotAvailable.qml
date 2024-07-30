/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    spacing: Kirigami.Units.largeSpacing
    // This is inside a Loader that is fillW/fillH: true
    // pad with "spacers" top/bottom to force centering
    Item { Layout.fillHeight: true }

    Kirigami.PlaceholderMessage {
        icon.name: "package-available-locked"
        text: compLoader.info
        explanation: i18nc("@info:status", "This feature is not yet available (%1)", compLoader.selector)
        Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
    }

    Item { Layout.fillHeight: true }
}
