// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

import QtQml 2.12
import QtQuick
import QtQuick.Layouts 1.3
import QtQuick.Controls as QQC2
import org.kde.kirigami 2.14 as Kirigami

import org.kde.kcmutils as KCMUtils
import org.kde.kitemmodels 1.0
import org.kcm.firewall 1.0

KCMUtils.ScrollViewKCM {
    id: root

    property QtObject model
    property var columns: []
    property alias emptyListText: emptyListLabel.text

    property QtObject currentJob: null

    property var blacklistRuleFactory
    property var blacklistColumns: []
    property string blacklistRuleSuccessMessage

    property int sortColumn: 0
    property int sortOrder: Qt.AscendingOrder

    property alias filterPlaceholderText: searchField.placeholderText
    property var filterColumns: []

    property var errorMessage: modelErrorMessage

    KSortFilterProxyModel {
        id: proxyModel
        sourceModel: root.model
        sortColumn: root.sortColumn
        sortOrder:  root.sortOrder

        function filterCb(source_row, source_parent) {
            const query = searchField.text.toLocaleLowerCase();
            const columns = filterColumns;

            const modelType = getModelType();

            for (var i = 0, length = columns.length; i < length; ++i) {
                const idx = sourceModel.index(source_row,  columns[i], source_parent);
                const data = String(sourceModel.data(idx) || "").toLocaleLowerCase();

                if (data.includes(query)) {
                    return true;
                }
            }

            return false;
        }

        filterRowCallback: searchField.length > 0 && filterColumns.length > 0 ? filterCb : null

        filterColumnCallback: (source_column, source_parent) => {
            return columns.map(column => column.column).includes(source_column)
        }
    }

    function getModelType() {
        // can this be done generically? :(
        if (root.model instanceof LogListModel) {
            return LogListModel;
        } else if (root.model instanceof ConnectionsModel) {
            return ConnectionsModel;
        }
        return null;
    }

    function blacklistRow(row) {
        // FIXME why does TableView does that? :(
        // Unfortunately it also casts to 0, so the resulting model index is deemed valid
        if (row === undefined) {
            return;
        }

        console.log("Accessing blacklist row", row);
        const idx = proxyModel.index(row, 0);
        const columns = blacklistColumns;
        const args = columns.map((column) => {
            return proxyModel.data(proxyModel.index(row, column));
        });

        if (args[0] === undefined) {
            console.log("Error, a model refresh happened when you tried to blacklist a connection.");
            return;
        }

        const rule = blacklistRuleFactory(...args);
        const job = kcm.client.addRule(rule);

        currentJob = job;
        ruleCreationErrorMessage.visible = false;
        console.log(...args);

        job.result.connect(function() {
            currentJob = null;

            if (job.error) {
                if (job.error !== 4) { // FIXME magic number
                    let indexError = job.errorString.indexOf("ERROR:");
                    let errorStrings = job.errorString.substring(indexError);

                    console.log(errorStrings);
                    ruleCreationErrorMessage.text = i18n("Error creating rule: %1", errorStrings);
                    ruleCreationErrorMessage.visible = true;
                }
                return;
            }

            if (blacklistRuleSuccessMessage) {
                kcm.showPassiveNotification(blacklistRuleSuccessMessage);
            }
        });
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: modelErrorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            showCloseButton: true

            Connections {
                target: root.model
                function onShowErrorMessage(message) {
                    modelErrorMessage.text = message;
                    modelErrorMessage.visible = true;
                }
            }
        }

        Kirigami.InlineMessage {
            id: ruleCreationErrorMessage
            type: Kirigami.MessageType.Error
            Layout.fillWidth: true
            showCloseButton: true
        }

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            onTextChanged: proxyModel.invalidateFilter();
            enabled: root.model.count > 0
            visible: root.filterColumns.length > 0
        }
    }

    QQC2.HorizontalHeaderView {
        id: horizontalHeader
        visible: tableView.rows > 0
        model: KColumnHeadersModel {
            sourceModel: proxyModel
            sortColumn: root.sortColumn
        }
        selectionModel: ItemSelectionModel{}
        syncView: tableView
        Layout.fillWidth: true

        TapHandler {
            acceptedButtons: Qt.LeftButton

            onTapped: (eventPoint, button) => {
                let cell = horizontalHeader.cellAtPosition(eventPoint.position)
                if (cell.x == root.sortColumn) {
                    root.sortOrder = header.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder
                } else {
                    root.sortOrder = Qt.AscendingOrder
                }
                root.sortColumn = cell.x
            }
        }
    }

    view: TableView {
        id: tableView
        anchors.fill: parent
        topMargin: horizontalHeader.height
        alternatingRows: true

        selectionModel: ItemSelectionModel {}
        selectionBehavior: TreeView.SelectRows
        function selectRelative(delta) {
            var nextRow = selectionModel.currentIndex.row + delta
            if (nextRow < 0) {
                nextRow = 0
            }
            if (nextRow >= rows) {
                nextRow = rows - 1
            }
            var index = model.index(nextRow, selectionModel.currentIndex.column)
            selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows)
        }
        Keys.onUpPressed: selectRelative(-1)
        Keys.onDownPressed: selectRelative(1)

        model: proxyModel
        columnWidthProvider: (column) => {
            let explicitWidth = explicitColumnWidth(column)
            if (explicitWidth > 0) {
                return explicitWidth
            }
            let sourceColumn = proxyModel.mapToSource(proxyModel.index(0, column)).column
            let width = root.columns.find(c => c.column == sourceColumn).width
            return width
        }
        clip: true
        delegate: QQC2.ItemDelegate {
            required property var model
            required property bool selected
            required property bool current
            text: model.display
            highlighted: selected || current
            onClicked: {
                tableView.selectionModel.setCurrentIndex(tableView.model.index(model.row, model.column), ItemSelectionModel.Rows | ItemSelectionModel.ClearAndSelect)
            }
        }
    }

    Kirigami.PlaceholderMessage {
        id: emptyListLabel
        parent: tableView.parent
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        visible: root.model.count === 0 && !root.model.busy && !modelErrorMessage.visible
    }

    QQC2.BusyIndicator {
        parent: tableView.parent
        anchors.centerIn: parent
        // Show busy spinner only on initial population and not while an error is shown
        running: root.model.count === 0 && root.model.busy && !modelErrorMessage.visible
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }

        InlineBusyIndicator {
            horizontalAlignment: Qt.AlignRight
            running: root.currentJob
        }

        QQC2.Button {
            text: i18n("Blacklist Connection")
            icon.name: "network-disconnect"
            // HACK TableView lets us select a fake zero index when view is empty...
            enabled: tableView.selectionModel.currentIndex  && model.count > 0 && !root.currentJob
            onClicked: blacklistRow(tableView.selectionModel.currentIndex.row)
        }
    }
}
