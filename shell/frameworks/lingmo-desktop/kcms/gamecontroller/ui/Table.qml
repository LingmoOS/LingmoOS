/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

Rectangle {
    id: root

    property alias model: buttonTable.model
    property string textRole: "display"

    border {
        color: LingmoUI.ColorUtils.tintWithAlpha(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.2)
        width: 1
    }

    radius: LingmoUI.Units.cornerRadius
    color: "transparent"

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: false

    QQC2.HorizontalHeaderView {
        id: buttonHorizontalHeader

        anchors {
            left: buttonTable.left
            top: parent.top
            topMargin: 1
        }

        syncView: buttonTable
        clip: true
    }

    QQC2.VerticalHeaderView {
        id: buttonVerticalHeader

        anchors {
            top: buttonTable.top
            left: parent.left
            leftMargin: 1
        }

        syncView: buttonTable
        clip: true
    }

    TableView {
        id: buttonTable

        anchors {
            left: buttonVerticalHeader.right
            top: buttonHorizontalHeader.bottom
            right: parent.right
            rightMargin: 1
            bottom: parent.bottom
            bottomMargin: 1
        }

        columnSpacing: 0
        rowSpacing: 1
        clip: true
        resizableRows: false
        resizableColumns: false
        alternatingRows: false
        boundsBehavior: Flickable.StopAtBounds
        selectionBehavior: TableView.SelectionDisabled
        pointerNavigationEnabled: false

        delegate: QQC2.ItemDelegate {
            implicitWidth: TableView.view.width

            text: model[root.textRole]

            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            LingmoUI.Theme.inherit: false
        }

        QQC2.ScrollBar.vertical: QQC2.ScrollBar {}
        QQC2.ScrollBar.horizontal: QQC2.ScrollBar {}
    }
}
