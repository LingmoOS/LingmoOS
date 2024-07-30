/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.ksysguard.formatter as Formatter
import org.kde.ksysguard.process as Process

import org.kde.kitemmodels as KItemModels

HorizontalHeaderView {
    id: header

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    property alias view: header.syncView

    property int sortColumn: 0
    property int sortOrder: Qt.AscendingOrder
    property string sortName
    onSortNameChanged: updateSelectedColumn()

    property string idRole: "Attribute"

    signal sort(int column, int order)
    signal resize(int column, real width)
    signal contextMenuRequested(int column, string id, point position)

    resizableColumns: true
    clip: true
    // Workaround for QTBUG-122560
    boundsBehavior: Flickable.StopAtBounds

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    onSortColumnChanged: sort(sortColumn, sortOrder)

    Component.onCompleted: Qt.callLater(updateSelectedColumn)

    function updateSelectedColumn() {
        let roleIndex = headerModel.KItemModels.KRoleNames.role(idRole)
        for (let i = 0; i < headerModel.rowCount(); ++i) {
            let index = headerModel.index(i, 0)
            if (headerModel.data(index, roleIndex) == sortName) {
                header.sortColumn = i
                header.sort(i, header.sortOrder)
                break
            }
        }
    }

    model: KItemModels.KColumnHeadersModel {
        id: headerModel
        sourceModel: header.view.model
        sortColumn: header.sortColumn
        sortOrder: header.sortOrder
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: (eventPoint, button) => {
            let cell = header.cellAtPosition(eventPoint.position)
            if (cell.x == header.sortColumn) {
                header.sortOrder = header.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder
                header.sort(header.sortColumn, header.sortOrder)
            } else {
                header.sortOrder = model.Unit == Formatter.Units.UnitNone || model.Unit == Formatter.Units.UnitInvalid ? Qt.AscendingOrder : Qt.DescendingOrder
                header.sortName = headerModel.data(headerModel.index(cell.x, 0), headerModel.KItemModels.KRoleNames.role(header.idRole))
                header.sortColumn = cell.x
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton

        gesturePolicy: TapHandler.ReleaseWithinBounds

        onTapped: (eventPoint, button) => {
            let cell = header.cellAtPosition(eventPoint.position)
            let columnId = header.modelIndex(cell).data(headerModel.KItemModels.KRoleNames.role(idRole));
            header.contextMenuRequested(cell.x, columnId, eventPoint.scenePosition)
        }
    }
}
