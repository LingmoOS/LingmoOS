/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.kitemmodels as ItemModels

import org.kde.kirigami as Kirigami
import org.kde.ksysguard.table as Table

FocusScope {
    id: root

    property var model
    property alias view: tableView
    property alias delegate: tableView.delegate

    property alias sortColumn: heading.sortColumn
    property alias sortOrder: heading.sortOrder
    property alias sortName: heading.sortName
    property alias idRole: heading.idRole

    // Column widths in fractions of the entire view width
    // Since column sizes are relative to a given view width, the widths of individual
    // columns need to be expressed as a fraction of the entire width. columnWidths
    // and defaultColumnWidth both store these fractions. Note that minimumColumnWidth
    // is expressed as a pixel value since we do not want that to scale with view width.
    property var columnWidths: []
    property real defaultColumnWidth: 0.1
    property real minimumColumnWidth: Kirigami.Units.gridUnit * 4

    readonly property alias headerHeight: heading.height

    readonly property alias selection: tableView.selectionModel
    readonly property alias rows: tableView.rows
    readonly property alias columns: tableView.columns

    signal contextMenuRequested(var index, point position)
    signal headerContextMenuRequested(int column, string columnId, point position)
    signal sort(int column, int order)

    Layout.minimumHeight: heading.height + root.rowHeight * 3

    clip: true

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    TableViewHeader {
        id: heading

        view: tableView

        width: scrollView.width

        onSort: (column, order) => {
            root.sort(column, order)
        }

        onResize: (column, width) => {
            root.setColumnWidth(column, width)
        }

        onContextMenuRequested:(column, columnId, position) => {
            root.headerContextMenuRequested(column, columnId, position)
        }
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.topMargin: heading.height

        property real innerWidth: LayoutMirroring.enabled ? width - leftPadding : width - rightPadding

        background: Rectangle { color: Kirigami.Theme.backgroundColor; Kirigami.Theme.colorSet: Kirigami.Theme.View }

        TreeView {
            id: tableView
            anchors.left: parent.left
            selectionModel: ItemSelectionModel {
                id: selectionModel
                model: tableView.model
            }
            property int hoveredRow: -1
            property int sortColumn: root.sortColumn

            // FIXME Until Tableview correctly reverses its columns, see QTBUG-90547
            model: root.model
            Binding on model {
                when: scrollView.LayoutMirroring.enabled
                value: Table.ReverseColumnsProxyModel {
                    sourceModel: root.model
                }
            }

            activeFocusOnTab: true

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            selectionBehavior: TableView.SelectRows
            selectionMode: TableView.ExtendedSelection

            onCurrentRowChanged: {
                // Workaround for QTBUG-114999
                if (!selectionModel.hasSelection) {
                    selectionModel.select(index(currentRow, 0), ItemSelectionModel.Select | ItemSelectionModel.Rows)
                }
            }

            columnWidthProvider: function(index) {
                let column = index
                // FIXME Until Tableview correctly reverses its columns, see QTBUG-90547
                if (LayoutMirroring.enabled) {
                    column = root.columnWidths.length - index - 1
                }

                // Resizing sets the explicit column width and has no other trigger. If
                // we don't make use of that value we can't resize. So read the value,
                // convert it to a fraction of total width and write it back to
                // columnWidths, then clear the explicit column width again so that
                // resizing updates the column width properly. This isn't the prettiest
                // of solutions but at least makes things work the way we want.
                let explicitWidth = explicitColumnWidth(index)
                if (explicitWidth >= 0) {
                    let w = explicitWidth / width
                    root.columnWidths[column] = w
                    root.columnWidthsChanged()
                    clearColumnWidths()
                }

                let columnWidth = root.columnWidths[column]
                return Math.max(Math.floor((columnWidth ?? root.defaultColumnWidth) * scrollView.innerWidth), root.minimumColumnWidth)
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.ReleaseWithinBounds

                onTapped: (eventPoint, button) => {
                    let cell = tableView.cellAtPosition(eventPoint.pressPosition);
                    let index = tableView.index(cell.y, cell.x)

                    if (!tableView.selectionModel.isSelected(index)) {
                        tableView.selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows)
                    }

                    root.contextMenuRequested(tableView.index(cell.y, cell.x), eventPoint.globalPressPosition)
                }
            }
        }
    }

    SelectionRectangle {
        target: tableView

        selectionMode: SelectionRectangle.Drag
        topLeftHandle: null
        bottomRightHandle: null
    }
}

