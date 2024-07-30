/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

// everything like in battery applet, but slightly bigger
RowLayout {
    property alias iconSource: iconItem.source
    property alias text: label.text

    spacing: Kirigami.Units.smallSpacing * 2

    Kirigami.Icon {
        id: iconItem
        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        visible: valid
    }

    PlasmaComponents3.Label {
        id: label
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(Kirigami.Units.gridUnit * 20, implicitWidth)
        font: Kirigami.Theme.smallFont
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        maximumLineCount: 4
    }
}
