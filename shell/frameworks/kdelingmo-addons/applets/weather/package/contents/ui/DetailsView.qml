/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents

ColumnLayout {
    id: root

    required property var model

    GridLayout {
        Layout.alignment: Qt.AlignCenter

        rows: labelRepeater.count
        flow: GridLayout.TopToBottom
        rowSpacing: LingmoUI.Units.smallSpacing

        Repeater {
            id: labelRepeater
            model: root.model
            delegate: LingmoComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                text: modelData.label
                textFormat: Text.PlainText
            }
        }

        Repeater {
            model: root.model
            delegate: LingmoComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                text: modelData.text
                textFormat: Text.PlainText
            }
        }
    }
}
