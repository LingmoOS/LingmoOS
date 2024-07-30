/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI

RowLayout {
    property alias iconSource: iconItem.source
    property alias text: label.text

    spacing: LingmoUI.Units.smallSpacing

    LingmoUI.Icon {
        id: iconItem
        Layout.preferredWidth: LingmoUI.Units.iconSizes.small
        Layout.preferredHeight: LingmoUI.Units.iconSizes.small
        visible: valid
    }

    LingmoComponents3.Label {
        id: label
        Layout.fillWidth: true
        font: LingmoUI.Theme.smallFont
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        maximumLineCount: 4
    }
}
