/*
    SPDX-FileCopyrightText: 2020 Ismael Asensio <isma.af@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmoui.delegates as KD

// External item required to make LingmoUI.ListItemDragHandle work
Item {
    id: delegate

    property ListView view

    implicitHeight : dictItem.implicitHeight

    signal removed(int index)
    signal moveRequested(int oldIndex, int newIndex)

    LingmoUI.SwipeListItem {
        id: dictItem

        // Don't need highlight, hover, or pressed effects
        highlighted: false
        hoverEnabled: false
        down: false

        text: model.id
        Accessible.description: model.description

        contentItem: RowLayout {
            LingmoUI.ListItemDragHandle {
                listItem: dictItem
                listView: delegate.view
                onMoveRequested: (oldIndex, newIndex) => {
                    delegate.moveRequested(oldIndex, newIndex);
                }
            }

            KD.TitleSubtitle {
                Layout.fillWidth: true

                selected: dictItem.highlighted

                title: dictItem.text
                subtitle: dictItem.Accessible.description
            }
        }

        actions: [
            LingmoUI.Action {
                text: i18n("Delete")
                icon.name: "entry-delete"
                onTriggered: {
                    delegate.removed(index);
                }
            }
        ]
    }
}
