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

ApplicationWindow {
    width: 500
    height: 500

    Rectangle {
        anchors.centerIn: parent
        width: 300
        height: 200
        border.width: 2

        Charts.LineChart {
            anchors.fill: parent

            colorSource: Charts.ArraySource { array: ["red", "green", "blue"] }
            nameSource: Charts.ArraySource { array: ["First", "Second", "Third"] }

            valueSources: [
                Charts.ArraySource { array: [1, 2, 2, 1] },
                Charts.ArraySource { array: [2, 5, 2, 5] },
                Charts.ArraySource { array: [5, 4, 3, 4] }
            ]
        }
    }
}
//! [example]
