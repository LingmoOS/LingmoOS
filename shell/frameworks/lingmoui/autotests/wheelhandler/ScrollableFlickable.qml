/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

ContentFlickable {
    id: flickable
    leftMargin: QQC2.ScrollBar.vertical && QQC2.ScrollBar.vertical.visible && QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
    rightMargin: QQC2.ScrollBar.vertical && QQC2.ScrollBar.vertical.visible && !QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
    bottomMargin: QQC2.ScrollBar.horizontal && QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.height : 0
    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        parent: flickable.parent
        anchors.right: flickable.right
        anchors.top: flickable.top
        anchors.bottom: flickable.bottom
        anchors.bottomMargin: flickable.QQC2.ScrollBar.horizontal ? flickable.QQC2.ScrollBar.horizontal.height : anchors.margins
        active: flickable.QQC2.ScrollBar.vertical.active
    }
    QQC2.ScrollBar.horizontal: QQC2.ScrollBar {
        parent: flickable.parent
        anchors.left: flickable.left
        anchors.right: flickable.right
        anchors.bottom: flickable.bottom
        anchors.rightMargin: flickable.QQC2.ScrollBar.vertical ? flickable.QQC2.ScrollBar.vertical.width : anchors.margins
        active: flickable.QQC2.ScrollBar.horizontal.active
    }
}
