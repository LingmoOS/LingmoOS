/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.qqc2desktopstyle.private as StylePrivate

T.VerticalHeaderView {
    id: controlRoot

    // The contentWidth of TableView will be zero at start-up, until the delegate
    // items have been loaded. This means that even if the implicit width of
    // VerticalHeaderView should be the same as the content width in the end, we
    // need to ensure that it has at least a width of 1 at start-up, otherwise
    // TableView won't bother loading any delegates at all.
    implicitWidth: Math.max(1, contentWidth)
    implicitHeight: syncView ? syncView.height : 0

    delegate: StylePrivate.StyleItem {
        required property var model
        required property int row
        readonly property string headerPosition: {
            if (controlRoot.rows === 1) {
                return "only";
            } else if (model.row == 0) {
                return "beginning";
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
            return selectionModel.rowIntersectsSelection(model.row)
        }
        properties: {
            "headerpos": headerPosition,
            "textalignment": Text.AlignVCenter | Text.AlignHCenter,
            "orientation": Qt.Vertical
        }
    }

    StylePrivate.StyleItem {
        parent: controlRoot
        anchors.fill: parent
        anchors.topMargin: controlRoot.contentHeight
        z: -1
        elementType: "header"
        properties: {
            "headerpos": "end",
            "orientation": Qt.Vertical
        }
    }
}
