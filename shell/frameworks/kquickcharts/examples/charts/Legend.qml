/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

ChartPage {
    title: "Legend"

    chart: Kirigami.AbstractCard {
        anchors.centerIn: parent
        width: parent.width
        height: 400

        contentItem: Item {
            Charts.PieChart {
                id: chart
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: legend.top
                    bottomMargin: Kirigami.Units.largeSpacing
                }

                valueSources: [
                    Charts.ModelSource { roleName: "value1"; model: itemModel },
                    Charts.ModelSource { roleName: "value2"; model: itemModel },
                    Charts.ModelSource { roleName: "value3"; model: itemModel }
                ]

                colorSource: Charts.ColorGradientSource {
                    baseColor: Kirigami.Theme.highlightColor
                    itemCount: itemModel.count * 3
                }
                nameSource: Charts.ArraySource {
                    array: {
                        let result = Array()
                        for (let i = 0; i < itemModel.count * 3; ++i) {
                            if (i < itemModel.count) {
                                result.push(itemModel.get(i).label + " Value 1")
                            } else if (i < itemModel.count * 2) {
                                result.push(itemModel.get(i % itemModel.count).label + " Value 2")
                            } else {
                                result.push(itemModel.get(i % itemModel.count).label + " Value 3")
                            }
                        }
                        return result
                    }
                }

                indexingMode: Charts.Chart.IndexAllValues

                highlight: legend.highlightedIndex
            }

            ChartsControls.Legend {
                id: legend

                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                height: Math.min(implicitHeight, parent.height * 0.3);

                chart: chart
                highlightEnabled: true
            }
        }
    }

    itemModel: ListModel {
        id: itemModel
        dynamicRoles: true

        Component.onCompleted: {
            append({label: "Item 1", value1: 10, value2:  9, value3:  3})
            append({label: "Item 2", value1: 10, value2:  5, value3: 17})
            append({label: "Item 3", value1: 10, value2: 16, value3:  8})
            append({label: "Item 4", value1: 10, value2: 12, value3: 11})
            append({label: "Item 5", value1: 10, value2:  8, value3: 11})
        }
    }

    itemDelegate: ItemDelegate {
        width: ListView.view.width
        contentItem: RowLayout {
            Label { text: "Label" }
            TextField {
                Layout.preferredWidth: 75
                text: model.label;
                onEditingFinished: itemModel.setProperty(index, "label", text)
            }
            Label { text: "Value 1" }
            SpinBox {
                Layout.preferredWidth: 75
                from: -10000; to: 10000;
                stepSize: 1;
                value: model.value1;
                onValueModified: itemModel.setProperty(index, "value1", value)
            }
            Label { text: "Value 2" }
            SpinBox {
                Layout.preferredWidth: 75
                from: -10000; to: 10000;
                stepSize: 1;
                value: model.value2;
                onValueModified: itemModel.setProperty(index, "value2", value)
            }
            Label { text: "Value 3" }
            SpinBox {
                Layout.preferredWidth: 75
                from: -10000; to: 10000;
                stepSize: 1;
                value: model.value3;
                onValueModified: itemModel.setProperty(index, "value3", value)
            }
        }
    }

    itemEditorActions: [
        Kirigami.Action {
            text: "Add Item"
            onTriggered: itemModel.append({label: "Item " + (itemModel.count + 1), value1: 10, value2: 10, value3: 10})
        },

        Kirigami.Action {
            text: "Remove Last"
            onTriggered: itemModel.remove(itemModel.count - 1)
        }
    ]
}
