/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Kirigami.Page {
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
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit
            Layout.topMargin: Kirigami.Units.gridUnit
        }

        ColumnLayout {
            id: controlsArea

            Layout.fillHeight: false
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
        }

        Kirigami.Separator { Layout.fillWidth: true; visible: itemEditor.visible }

        Kirigami.InlineViewHeader {
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
                color: Kirigami.Theme.backgroundColor
            }

            HoverHandler {
                onHoveredChanged: page.itemEditorHovered = hovered
            }
        }
    }
}
