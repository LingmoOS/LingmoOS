/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.MenuBar {
    id: controlRoot

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Header
    LingmoUI.Theme.inherit: false

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    delegate: MenuBarItem {}

    contentItem: Row {
        spacing: controlRoot.spacing
        Repeater {
            model: controlRoot.contentModel
        }
    }

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
    }
}
