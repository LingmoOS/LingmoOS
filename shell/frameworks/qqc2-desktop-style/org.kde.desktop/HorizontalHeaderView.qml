/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.qqc2desktopstyle.private as StylePrivate

T.HorizontalHeaderView {
    id: controlRoot

    implicitWidth: syncView ? syncView.width : 0
    // The contentHeight of TableView will be zero at start-up, until the delegate
    // items have been loaded. This means that even if the implicit height of
    // HorizontalHeaderView should be the same as the content height in the end, we
    // need to ensure that it has at least a height of 1 at start-up, otherwise
    // TableView won't bother loading any delegates at all.
    implicitHeight: Math.max(1, contentHeight)

    delegate: StylePrivate.StyleItem {
        required property var model
        required property int column
        readonly property string headerPosition: {
            if (controlRoot.columns === 1) {
                return "only";
            } else if (model.column === 0) {
                return LayoutMirroring.enabled ? "end" : "beginning";
            } else {
                return "middle";
            }
        }

        text: model[controlRoot.textRole]
        elementType: "header"
        on: {
            let selectionModel = controlRoot.selectionModel
            if (!selectionModel && controlRoot.syncView) {
                if (controlRoot.syncView.selectionModel && controlRoot.syncView.model == controlRoot.model) {
                    selectionModel = controlRoot.syncView.selectionModel
                }
            }
            if (!selectionModel) {
                return false
            }

            // This line is for property bindings
            void(selectionModel.selectedIndexes);
            return selectionModel.columnIntersectsSelection(model.column)
        }
        //FIXME: this is not usable as we don't have ways to query the sort column
        //activeControl: orderQuery ? (filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? "down" : "up") : ""
        activeControl: {
            if (model.sort !== undefined) {
                return model.sort == Qt.AscendingOrder ? "down" : "up"
            }
            return ""
        }
        properties: {
            "headerpos": headerPosition,
            "textalignment": Text.AlignVCenter | Text.AlignHCenter,
            "orientation": Qt.Horizontal
        }
    }

    StylePrivate.StyleItem {
        parent: controlRoot
        anchors.fill: parent
        anchors.leftMargin: controlRoot.contentWidth
        z: -1
        elementType: "header"
        properties: {
            "headerpos": LayoutMirroring.enabled ? "beginning" : "end",
            "orientation": Qt.Horizontal
        }
    }
}
