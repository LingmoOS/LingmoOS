/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

LingmoUI.ActionToolBar {
    id: control

    property alias addActions: addAction.children
    property list<Action> extraActions

    property bool single: false
    property bool addVisible: true

    property int moveAxis: Qt.YAxis
    property Item moveTarget

    property Page page

    signal moved(int from, int to)
    signal removeClicked()

    actions: [addAction, moveAction, removeAction].concat(Array.prototype.map.call(extraActions, i => i))

    flat: false

    LingmoUI.Action {
        id: addAction
        text: i18nc("@action", "Add")
        icon.name: "list-add"
        visible: control.addVisible
    }

    LingmoUI.Action {
        id: moveAction

        displayComponent: MoveButton {
            axis: control.moveAxis
            target: control.moveTarget
            onMoved: control.moved(from, to)
            enabled: !control.single
            page: control.page
        }
    }

    LingmoUI.Action {
        id: removeAction
        enabled: !control.single
        text: i18nc("@action", "Remove")
        icon.name: "edit-delete"
        onTriggered: control.removeClicked()
    }
}
