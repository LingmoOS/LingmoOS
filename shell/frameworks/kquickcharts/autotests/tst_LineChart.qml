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
    name: "Line Chart Tests"

    width: 400
    height: 400
    visible: true
    when: windowShown

    Component {
        id: minimal
        Charts.LineChart { }
    }

    Component {
        id: simple
        Charts.LineChart {
            width: 200
            height: 200
            nameSource: Charts.ArraySource { array: ["Test 1", "Test 2", "Test 3"] }
            colorSource: Charts.ArraySource { array: ["red", "green", "blue"] }
            valueSources: Charts.ArraySource { array: [1, 2, 3, 4, 5] }
        }
    }

    function test_create_data() {
        return [
            { tag: "minimal", component: minimal },
            { tag: "simple", component: simple }
        ]
    }

    function test_create(data) {
        var item = createTemporaryObject(data.component, testCase)
        verify(item)
        verify(waitForRendering(item))
    }
}
