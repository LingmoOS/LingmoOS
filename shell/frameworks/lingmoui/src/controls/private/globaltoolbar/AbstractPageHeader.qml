/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.AbstractApplicationHeader {
    id: root
    // anchors.fill: parent
    property Item container
    property bool current

    minimumHeight: pageRow ? pageRow.globalToolBar.minimumHeight : LingmoUI.Units.iconSizes.medium + LingmoUI.Units.smallSpacing * 2
    maximumHeight: pageRow ? pageRow.globalToolBar.maximumHeight : minimumHeight
    preferredHeight: pageRow ? pageRow.globalToolBar.preferredHeight : minimumHeight

    separatorVisible: pageRow ? pageRow.globalToolBar.separatorVisible : true

    LingmoUI.Theme.colorSet: pageRow ? pageRow.globalToolBar.colorSet : LingmoUI.Theme.Header

    leftPadding: pageRow
        ? Math.min(
            width / 2,
            Math.max(
                (page.title.length > 0 ? pageRow.globalToolBar.titleLeftPadding : 0),
                Qt.application.layoutDirection === Qt.LeftToRight
                    ? Math.min(pageRow.globalToolBar.leftReservedSpace,
                        pageRow.LingmoUI.ScenePosition.x
                        - page.LingmoUI.ScenePosition.x
                        + pageRow.globalToolBar.leftReservedSpace)
                        + LingmoUI.Units.smallSpacing
                    : Math.min(pageRow.globalToolBar.leftReservedSpace,
                        -pageRow.width
                        + pageRow.LingmoUI.ScenePosition.x
                        + page.LingmoUI.ScenePosition.x
                        + page.width
                        + pageRow.globalToolBar.leftReservedSpace)
                        + LingmoUI.Units.smallSpacing))
        : LingmoUI.Units.smallSpacing
    rightPadding: pageRow
        ? Math.max(0,
            Qt.application.layoutDirection === Qt.LeftToRight
            ? (-pageRow.width
                - pageRow.LingmoUI.ScenePosition.x
                + page.width
                + page.LingmoUI.ScenePosition.x
                + pageRow.globalToolBar.rightReservedSpace)
            : (pageRow.LingmoUI.ScenePosition.x
                - page.LingmoUI.ScenePosition.x
                + pageRow.globalToolBar.rightReservedSpace))
        : 0
}
