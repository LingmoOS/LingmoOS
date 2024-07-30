/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

LingmoUI.Page {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property alias chart: chartArea.data
    property alias controls: controlsArea.data

    property QtObject itemModel
    property alias itemDelegate: itemView.delegate
    property bool itemEditorHovered: false
    property list<Action> itemEditorActions

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            id: chartArea

            Layout.fillWidth: true
            Layout.preferredHeight: 400
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.leftMargin: LingmoUI.Units.gridUnit
            Layout.rightMargin: LingmoUI.Units.gridUnit
            Layout.topMargin: LingmoUI.Units.gridUnit
        }

        ColumnLayout {
            id: controlsArea

            Layout.fillHeight: false
            Layout.leftMargin: LingmoUI.Units.gridUnit
            Layout.rightMargin: LingmoUI.Units.gridUnit
            Layout.topMargin: LingmoUI.Units.largeSpacing
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
        }

        LingmoUI.Separator { Layout.fillWidth: true; visible: itemEditor.visible }

        LingmoUI.InlineViewHeader {
            Layout.fillWidth: true
            visible: itemEditor.visible
            text: "Items"
            actions: page.itemEditorActions
        }

        ScrollView {
            id: itemEditor

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.itemModel != null

            ListView {
                id: itemView

                model: page.itemModel

                clip: true
            }

            background: Rectangle {
                color: LingmoUI.Theme.backgroundColor
            }

            HoverHandler {
                onHoveredChanged: page.itemEditorHovered = hovered
            }
        }
    }
}
