/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtTest

import org.kde.quickcharts as Charts

TestCase {
    id: testCase
    name: "Pie Chart Tests"

    width: 400
    height: 400
    visible: true
    when: windowShown

    Component {
        id: minimal
        Charts.PieChart { }
    }

    Component {
        id: simple
        Charts.PieChart {
            width: 200
            height: 200
            nameSource: Charts.ArraySource { array: ["Test 1", "Test 2", "Test 3"] }
            colorSource: Charts.ArraySource { array: ["red", "green", "blue"] }
            valueSources: Charts.ArraySource { array: [1, 2, 3, 4, 5] }
        }
    }

    Component {
        id: multiValue
        Charts.PieChart {
            width: 200
            height: 200
            nameSource: Charts.ArraySource { array: ["Test 1", "Test 2", "Test 3"] }
            colorSource: Charts.ArraySource { array: ["red", "green", "blue"] }
            valueSources: [
                Charts.ArraySource { array: [1, 2, 3, 4, 5] },
                Charts.ArraySource { array: [1, 2, 3, 4, 5] },
                Charts.ArraySource { array: [1, 2, 3, 4, 5] }
            ]
        }
    }

    Component {
        id: model
        Charts.PieChart {
            width: 200
            height: 200

            valueSources: [
                Charts.ModelSource {
                    model: ListModel {
                        id: listModel
                        ListElement { name: "Test 1"; color: "red"; value: 1 }
                        ListElement { name: "Test 2"; color: "green"; value: 2 }
                        ListElement { name: "Test 3"; color: "blue"; value: 3 }
                        ListElement { name: "Test 4"; color: "cyan"; value: 4 }
                        ListElement { name: "Test 5"; color: "magenta"; value: 5 }
                        ListElement { name: "Test 6"; color: "yellow"; value: 6 }
                    }
                }
            ]
            nameSource: Charts.ModelSource { model: listModel; roleName: "name" }
            colorSource: Charts.ModelSource { model: listModel; roleName: "color" }
        }
    }

    function test_create_data() {
        return [
            { tag: "minimal", component: minimal },
            { tag: "simple", component: simple },
            { tag: "multiValue", component: multiValue },
            { tag: "model", component: model }
        ]
    }

    function test_create(data) {
        var item = createTemporaryObject(data.component, testCase)
        verify(item)
        verify(waitForRendering(item))
    }
}

