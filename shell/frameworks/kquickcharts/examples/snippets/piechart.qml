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

Charts.PieChart {
    width: 400
    height: 300

    valueSources: Charts.ModelSource {
        roleName: "data";
        model: ListModel {
            id: listModel
            ListElement { data: 12 }
            ListElement { data: 19 }
            ListElement { data: 10 }
            ListElement { data: 13 }
            ListElement { data: 13 }
        }
    }
    colorSource: Charts.ArraySource { array: ["red", "green", "blue", "yellow", "cyan"] }
    nameSource: Charts.ArraySource { array: ["Red", "Green", "Blue", "Yellow", "Cyan"] }

    thickness: 50
}
//! [example]
