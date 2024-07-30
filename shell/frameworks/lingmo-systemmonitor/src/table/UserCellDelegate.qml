/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

TextCellDelegate {
    id: delegate

    property int uidColumn
    property var currentUser

    horizontalAlignment: Text.AlignHCenter

    background: CellBackground {
        view: delegate.TableView.view
        row: model.row;
        column: model.column
        color: {
            var uid = -1
            var m = delegate.TableView.view.model
            if ("sourceModel" in m) {
                var index = m.mapToSource(m.index(model.row, 0))
                index = m.sourceModel.index(index.row, delegate.uidColumn)
                uid = m.sourceModel.data(index)
            } else {
                uid = m.data(m.index(model.row, delegate.uidColumn))
            }

            if (uid >= 0 && (uid < 1000 || uid >= 65534)) {
                return Qt.darker(LingmoUI.Theme.alternateBackgroundColor, model.row % 2 == 0 ? 1.0 : 1.1)
            } else if (model.display == currentUser.loginName) {
                return Qt.rgba(
                    LingmoUI.Theme.highlightColor.r,
                    LingmoUI.Theme.highlightColor.g,
                    LingmoUI.Theme.highlightColor.b,
                    model.row % 2 == 0 ? 0.4 : 0.5
                )
            }
            return LingmoUI.Theme.backgroundColor
        }
    }
}
