/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

LingmoComponents.Page {
    id: root

    padding: LingmoUI.Units.largeSpacing

    component GridLabel : LingmoComponents.Label {
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignRight
    }

    component ColorItem : Rectangle {
        implicitWidth: LingmoUI.Units.gridUnit * 2
        implicitHeight: Math.round(LingmoUI.Units.gridUnit * 1.5)
        radius: 5
        border {
            color: root.LingmoUI.Theme.textColor
            width: 1
        }
    }

    contentItem: GridLayout {
        columns: 2
        columnSpacing: LingmoUI.Units.largeSpacing
        rowSpacing: LingmoUI.Units.largeSpacing

        LingmoUI.Heading {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            elide: Text.ElideRight
            level: 1
            text: "Theme"
        }

        LingmoComponents.Label {
            Layout.columnSpan: 2
            text: "This is the smallest readable Font."
            font: LingmoUI.Theme.smallFont
        }

        GridLabel {
            text: "textColor:"
        }
        ColorItem {
            color: LingmoUI.Theme.textColor
        }

        GridLabel {
            text: "Button textColor:"
        }
        ColorItem {
            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            color: LingmoUI.Theme.textColor
        }

        GridLabel {
            text: "highlightColor:"
        }
        ColorItem {
            color: LingmoUI.Theme.highlightColor
        }

        GridLabel {
            text: "View backgroundColor:"
        }
        ColorItem {
            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            color: LingmoUI.Theme.backgroundColor
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
