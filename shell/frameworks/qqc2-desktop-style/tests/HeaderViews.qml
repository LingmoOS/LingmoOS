/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3 as PC3
import Qt.labs.qmlmodels 1.0

GridLayout {
    id: root
    width: 500
    height: 300
    rows: 2
    columns: 2
    rowSpacing: 0
    columnSpacing: 0

    Item {}
    QQC2.HorizontalHeaderView {
        syncView: table
        Layout.fillWidth: true
    }
    QQC2.VerticalHeaderView {
        syncView: table
        Layout.fillWidth: true
    }
    TableView {
        id: table
        Layout.fillWidth: true
        Layout.fillHeight: true
        alternatingRows: true

        selectionModel: ItemSelectionModel {}

        model: TableModel {
            TableModelColumn { display: "A" }
            TableModelColumn { display: "B" }
            TableModelColumn { display: "C" }
            TableModelColumn { display: "D" }

            rows: [
                {
                    A: "A1",
                    B: "B1",
                    C: "C1",
                    D: "D1"
                },
                {
                    A: "A2",
                    B: "B2",
                    C: "C2",
                    D: "D2"
                },
                {
                    A: "A3",
                    B: "B3",
                    C: "C3",
                    D: "D3"
                }
            ]
        }
        delegate: QQC2.ItemDelegate {
            required property var model
            required property bool selected
            text: model.display
            checkable: true
            checked: selected
            highlighted: checked
            onClicked: {
                table.selectionModel.select(table.model.index(model.row, model.column), checked ? ItemSelectionModel.Select : ItemSelectionModel.Deselect)
            }
        }
    }

}

