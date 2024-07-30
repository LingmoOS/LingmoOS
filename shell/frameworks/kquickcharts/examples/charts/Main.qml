/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: window
    title: "Charts Example"

    width: Kirigami.Units.gridUnit * 60
    height: Kirigami.Units.gridUnit * 40

    pageStack.initialPage: Kirigami.ScrollablePage {
        title: "Charts"

        ListView {

            model: ListModel {
                id: pagesModel
                ListElement { label: "Pie Chart"; file: "PieChart"; identifier: "pie" }
                ListElement { label: "Line Chart"; file: "LineChart"; identifier: "line" }
                ListElement { label: "Bar Chart"; file: "BarChart"; identifier: "bar" }
                ListElement { label: "History Proxy Source"; file: "HistoryProxySource"; identifier: "history" }
                ListElement { label: "Legend"; file: "Legend"; identifier: "legend" }
            }

            delegate: ItemDelegate {
                width: ListView.view.width

                text: model.label
                onClicked: {
                    let component = Qt.createComponent("org.kde.quickcharts.example", model.file)
                    applicationWindow().pageStack.push(component);
                }
            }
        }
    }

    Component.onCompleted: {
        if (__commandLinePage !== null) {
            for (var i = 0; i < pagesModel.count; ++i) {
                var item = pagesModel.get(i);
                if (item.identifier == __commandLinePage || item.label == __commandLinePage) {
                    window.pageStack.push(item.file);
                    return;
                }
            }
        }
    }
}
