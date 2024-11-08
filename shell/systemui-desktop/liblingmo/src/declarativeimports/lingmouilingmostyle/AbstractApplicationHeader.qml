/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDY-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmoui as LingmoUI

import "../../templates" as T
import "../../private" as LingmoUIPrivate

T.AbstractApplicationHeader {
    id: root

    LingmoUI.Theme.inherit: false
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Header

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
        LingmoUIPrivate.EdgeShadow {
            radius: LingmoUI.Units.gridUnit * 0.5
            visible: root.separatorVisible
            anchors {
                right: parent.right
                left: parent.left
                top: parent.bottom
            }
            edge: Qt.TopEdge
        }
    }
}
