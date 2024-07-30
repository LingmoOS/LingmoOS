/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3

LingmoComponents3.ItemDelegate {
    id: item

    Layout.fillWidth: true

    property alias subText: sublabel.text
    property alias iconItem: iconItem.children

    highlighted: activeFocus

    Accessible.name: `${text}${subText ? `: ${subText}` : ""}`

    onHoveredChanged: if (hovered) {
        if (ListView.view) {
            ListView.view.currentIndex = index;
        }
        forceActiveFocus();
    }

    contentItem: RowLayout {
        id: row

        spacing: LingmoUI.Units.smallSpacing

        Item {
            id: iconItem

            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
            Layout.minimumWidth: Layout.preferredWidth
            Layout.maximumWidth: Layout.preferredWidth
            Layout.minimumHeight: Layout.preferredHeight
            Layout.maximumHeight: Layout.preferredHeight
        }

        ColumnLayout {
            id: column
            Layout.fillWidth: true
            spacing: 0

            LingmoComponents3.Label {
                id: label
                Layout.fillWidth: true
                text: item.text
                textFormat: Text.PlainText
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
            }

            LingmoComponents3.Label {
                id: sublabel
                Layout.fillWidth: true
                textFormat: Text.PlainText
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                opacity: 0.6
                font: LingmoUI.Theme.smallFont
                visible: text !== ""
            }
        }
    }
}
