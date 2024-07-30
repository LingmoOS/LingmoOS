/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.lingmoui as LingmoUI
import org.kde.ksysguard.table as Table

TreeViewDelegate
{
    id: root

    readonly property bool rowHovered: root.TableView.view.hoveredRow == row

    text: model.display

    // Important: Don't remove this until QTBUG-84858 is resolved properly.
    Accessible.role: Accessible.Cell

    rightPadding: LingmoUI.Units.smallSpacing
    topPadding: LingmoUI.Units.smallSpacing
    bottomPadding: LingmoUI.Units.smallSpacing

    background: Rectangle {
        color: (row % 2 == 0 || selected) ? LingmoUI.Theme.backgroundColor : LingmoUI.Theme.alternateBackgroundColor

        Rectangle {
            anchors.fill: parent
            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Selection
            color: LingmoUI.Theme.backgroundColor
            opacity: 0.3
            visible: root.rowHovered
        }
    }

    onHoveredChanged: {
        if (hovered) {
            root.TableView.view.hoveredRow = root.row
        }
    }

    hoverEnabled: true
}
