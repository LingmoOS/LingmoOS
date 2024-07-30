/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.DialogButtonBox {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    spacing: LingmoUI.Units.smallSpacing

    topPadding: (parent as T.Control)?.topPadding ?? undefined
    leftPadding: (parent as T.Control)?.leftPadding ?? undefined
    rightPadding: (parent as T.Control)?.rightPadding ?? undefined
    bottomPadding: (parent as T.Control)?.bottomPadding ?? undefined

    alignment: Qt.AlignRight

    delegate: Button {
        width: Math.min(implicitWidth, control.width / control.count - control.rightPadding - control.spacing * (control.count-1))
    }

    contentItem: ListView {
        implicitWidth: contentWidth
        implicitHeight: LingmoUI.Units.gridUnit * 1.6

        model: control.contentModel
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
    }

    background: Item {}
}
