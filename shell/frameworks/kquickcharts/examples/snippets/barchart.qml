/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

//! [example]
import QtQuick
import QtQuick.Controls

import org.kde.quickcharts as Charts

Charts.BarChart {
    id: barChart

    width: 400
    height: 300

    valueSources: [
        Charts.ModelSource { roleName: "value1"; model: listModel },
        Charts.ModelSource { roleName: "value2"; model: listModel },
        Charts.ModelSource { roleName: "value3"; model: listModel }
    ]

    colorSource: Charts.ArraySource { array: ["red", "green", "blue"] }
    nameSource: Charts.ArraySource { array: ["Example 1", "Example 2", "Example 3"] }

    barWidth: 20

    ListModel {
        id: listModel

        ListElement { value1: 19; value2: 2; value3: 6 }
        ListElement { value1: 14; value2: 20; value3: 17 }
        ListElement { value1: 4; value2: 10; value3: 11 }
        ListElement { value1: 5; value2: 11; value3: 9 }
        ListElement { value1: 20; value2: 7; value3: 13 }
    }
}
//! [example]
