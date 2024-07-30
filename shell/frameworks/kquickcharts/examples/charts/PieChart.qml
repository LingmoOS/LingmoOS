/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.quickcharts as Charts

ChartPage {
    title: "Pie Chart"

    chart: Kirigami.AbstractCard {
        width: 600
        height: 400
        anchors.centerIn: parent

        Charts.PieChart {
            id: chart
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing;

            range.to: 150

            valueSources: Charts.ModelSource { roleName: "data"; model: pieModel }

            colorSource: Charts.ColorGradientSource {
                baseColor: Kirigami.Theme.highlightColor
                itemCount: pieModel.count
            }

            thickness: 20;
            filled: false;
            backgroundColor: "gray"
        }
    }

    controls: [
        RangeEditor { label: "Range"; range: chart.range },

        RowLayout {
            Label { text: "Thickness" }
            SpinBox { from: 0; to: chart.width / 2; value: chart.thickness; onValueModified: chart.thickness = value; }
            CheckBox { text: "Filled"; checked: chart.filled; onCheckedChanged: chart.filled = checked }
            CheckBox { text: "Smooth Ends"; checked: chart.smoothEnds; onCheckedChanged: chart.smoothEnds = checked }
            Label { text: "From Angle" }
            SpinBox { from: -360; to: 360; value: chart.fromAngle; onValueModified: chart.fromAngle = value; }
            Label { text: "To Angle" }
            SpinBox { from: -360; to: 360; value: chart.toAngle; onValueModified: chart.toAngle = value; }
        }
    ]

    itemModel: ListModel {
        id: pieModel;
        dynamicRoles: true;

        Component.onCompleted: {
            append({label: "Item 1", data: 50})
            append({label: "Item 2", data: 50})
            append({label: "Item 3", data: 50})
        }
    }

    itemDelegate: ItemDelegate {
        width: ListView.view.width
        onHoveredChanged: {
            if (hovered) {
                chart.highlight = index
            }
        }
        contentItem: RowLayout {
            Label {
                text: "Label"
            }
            TextField {
                Layout.preferredWidth: 200
                text: model.label;
                onEditingFinished: pieModel.setProperty(index, "label", text)
            }
            Label {
                text: "Value"
            }
            SpinBox {
                Layout.preferredWidth: 100
                from: 0; to: 10000;
                stepSize: 1;
                value: model.data;
                onValueModified: pieModel.setProperty(index, "data", value)
            }
        }
    }

    onItemEditorHoveredChanged: {
        if (!itemEditorHovered) {
            chart.highlight = undefined
        }
    }

    itemEditorActions: [
        Kirigami.Action {
            text: "Add Item"
            onTriggered: pieModel.append({label: "Item " + (pieModel.count + 1), data: 50})
        },

        Kirigami.Action {
            text: "Remove Last"
            onTriggered: pieModel.remove(pieModel.count - 1)
        }
    ]
}
