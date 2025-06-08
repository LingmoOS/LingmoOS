/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.1 as PlasmaCore

// everything like in battery applet, but slightly bigger
RowLayout {
    property alias iconSource: iconItem.source
    property alias text: label.text

    spacing: PlasmaCore.Units.smallSpacing * 2

    PlasmaCore.IconItem {
        id: iconItem
        Layout.preferredWidth: PlasmaCore.Units.iconSizes.medium
        Layout.preferredHeight: PlasmaCore.Units.iconSizes.medium
        visible: valid
    }

    PlasmaComponents3.Label {
        id: label
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(PlasmaCore.Units.gridUnit * 20, implicitWidth)
        font: PlasmaCore.Theme.smallestFont
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        maximumLineCount: 4
    }
}
