/*
 *   SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Qt.labs.qmlmodels

import org.kde.kitemmodels as KItemModels
import org.kde.quickcharts as Charts

import org.kde.ksysguard.formatter as Formatter
import org.kde.ksysguard.process as Process
import org.kde.ksysguard.table as Table

Table.BaseTableView {
    id: view

    property alias nameFilterString: rowFilter.filterString
    property alias processModel: processModel
    property alias viewMode: rowFilter.viewMode
    property bool flatList

    readonly property bool treeModeSupported: processModel.hasOwnProperty("flatList")

    property alias columnDisplay: displayModel.columnDisplay
    property var enabledColumns
    onEnabledColumnsChanged: processModel.updateEnabledAttributes()

    readonly property var selectedProcesses: {
        var result = []
        var rows = {}

        for (var i of selection.selectedIndexes) {
            if (rows[i.row] != undefined) {
                continue
            }

            rows[i.row] = true

            var index = rowFilter.mapToSource(i)

            var item = {}

            item.name = processModel.data(processModel.index(index.row, processModel.nameColumn, index.parent), Process.ProcessDataModel.ValueRole)
            item.pid = processModel.data(processModel.index(index.row, processModel.pidColumn, index.parent), Process.ProcessDataModel.ValueRole)
            item.username = processModel.data(processModel.index(index.row, processModel.usernameColumn, index.parent), Process.ProcessDataModel.ValueRole)

            result.push(item)
        }

        return result
    }

    idRole: "Attribute"

    onSort: (column, order) => rowFilter.sort(column, order)

    model: Table.ProcessSortFilterModel {
        id: rowFilter
        sourceModel: cacheModel
        hiddenAttributes: processModel.hiddenSensors
    }

    Table.ComponentCacheProxyModel {
        id: cacheModel
        sourceModel: displayModel

        component: Charts.HistoryProxySource {
            id: history
            source: Charts.ModelSource {
                model: history.Table.ComponentCacheProxyModel.model
                column: history.Table.ComponentCacheProxyModel.column
                roleName: "Value"
            }
            item: Table.ComponentCacheProxyModel.row
            maximumHistory: 10
            interval: 2000
            fillMode: Charts.HistoryProxySource.FillFromEnd
        }
    }

    Table.ColumnDisplayModel {
        id: displayModel
        sourceModel: processModel
        idRole: "Attribute"
    }

    // Temporary workaround to be able to run against Plasma 5.20,
    // which lacks the "flatList" property on ProcessDataModel. If
    // we assign it directly, the face will fail to load because
    // the property is missing. However, when using a Binding here
    // it will only produce a runtime warning.
    Binding {
        target: processModel
        property: "flatList"
        value: view.flatList
    }

    Process.ProcessDataModel {
        id: processModel
        onFlatListChanged:  Qt.callLater(view.view.expandRecursively, -1, -1)

        property int nameColumn
        property int pidColumn
        property int uidColumn
        property int usernameColumn

        property var requiredSensors: [
            "name",
            "pid",
            "uid",
            "username"
        ]
        property var hiddenSensors: []

        enabled: view.visible

        function updateEnabledAttributes() {
            var result = []
            for (let i of view.enabledColumns) {
                if (processModel.availableAttributes.includes(i)) {
                    result.push(i)
                }
            }

            var hidden = []
            for (let i of requiredSensors) {
                if (result.indexOf(i) == -1 && processModel.availableAttributes.includes(i)) {
                    result.push(i)
                    hidden.push(i)
                }
            }

            processModel.nameColumn = result.indexOf("name")
            processModel.pidColumn = result.indexOf("pid")
            processModel.uidColumn = result.indexOf("uid")
            processModel.usernameColumn = result.indexOf("username")
            processModel.hiddenSensors = hidden
            processModel.enabledAttributes = result
        }
    }

    delegate: DelegateChooser {
        role: "displayStyle"
        DelegateChoice {
            column: view.LayoutMirroring.enabled ? view.model.columnCount() - 1 : 0
            Table.FirstCellDelegate {
                id: delegate
                treeDecorationVisible: !view.flatList
                iconName: {
                    var index = treeView.index(model.row, 0);
                    index = rowFilter.mapToSource(index)
                    index = cacheModel.mapToSource(index);
                    index = displayModel.mapToSource(index);
                    index = processModel.index(index.row, processModel.nameColumn, index.parent);
                    return processModel.data(index).split(" ")[0].toLowerCase();
                }
            }
        }
        DelegateChoice {
            roleValue: "line"
            Table.LineChartCellDelegate {
                valueSources: model.cachedComponent != undefined ? model.cachedComponent : []
                maximum: rowFilter.data(treeView.index(model.row, model.column), Process.ProcessDataModel.Maximum)
            }

        }
        DelegateChoice {
            roleValue: "lineScaled"
            Table.LineChartCellDelegate {
                valueSources: model.cachedComponent != undefined ? model.cachedComponent : []
                maximum: rowFilter.data(treeView.index(model.row, model.column), Process.ProcessDataModel.Maximum)
                text: Formatter.Formatter.formatValue(parseInt(model.Value) / model.Maximum * 100, model.Unit)
            }

        }
        DelegateChoice {
            roleValue: "textScaled"
            Table.TextCellDelegate {
                text: Formatter.Formatter.formatValue(parseInt(model.Value) / model.Maximum * 100, model.Unit)
            }
        }
        DelegateChoice { Table.TextCellDelegate{} }
    }
}
