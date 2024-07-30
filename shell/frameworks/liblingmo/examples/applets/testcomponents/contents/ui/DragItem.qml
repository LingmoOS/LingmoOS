/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents

import org.kde.draganddrop as DragAndDrop

LingmoComponents.ItemDelegate {
    id: control

    width: parent?.width ?? 0
    //height: _h * 1.5

    LingmoUI.Icon {
        id: itemIcon

        width: _h
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: _h/2

        source: control.icon.name
    }

    LingmoComponents.Label {
        id: label

        anchors {
            verticalCenter: parent.verticalCenter
            left: itemIcon.right
            leftMargin: _h/2
            right: parent.right
            rightMargin: _h/2
        }

        text: control.text
    }
}
