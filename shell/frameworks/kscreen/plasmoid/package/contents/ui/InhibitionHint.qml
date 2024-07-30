/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI

// everything like in battery applet, but slightly bigger
RowLayout {
    property alias iconSource: iconItem.source
    property alias text: label.text

    spacing: LingmoUI.Units.smallSpacing * 2

    LingmoUI.Icon {
        id: iconItem
        Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
        Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
        visible: valid
    }

    LingmoComponents3.Label {
        id: label
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(LingmoUI.Units.gridUnit * 20, implicitWidth)
        font: LingmoUI.Theme.smallFont
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        maximumLineCount: 4
    }
}
