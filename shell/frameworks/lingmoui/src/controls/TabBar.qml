/*
 * Copyright (C) 2024 Lingmo OS Team <team@lingmo.org>
 *
 * Author: Lingmo OS Team <team@lingmo.org>
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */
import QtQuick 2.9
import QtQuick.Templates 2.2 as T
import Qt5Compat.GraphicalEffects
import org.kde.lingmoui as LingmoUI

T.TabBar {
    id: control

    implicitWidth: Math.max(background.implicitWidth, contentItem.implicitWidth + LingmoUI.Units.smallSpacing)
    implicitHeight: contentItem.implicitHeight

    spacing: 0

    contentItem: ListView {
        implicitWidth: contentWidth
        implicitHeight: control.contentModel.get(0).height

        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: 0
        highlightResizeDuration: 0
        highlightFollowsCurrentItem: true
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 48
        preferredHighlightEnd: width - 48

        highlight: Item {
            Rectangle {
                anchors {
                    fill: parent
                    margins: 2
                }
                color: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 2)
                                             : LingmoUI.Theme.secondBackgroundColor
                radius: LingmoUI.Theme.smallRadius

                layer.enabled: LingmoUI.Theme.darkMode
                layer.effect: DropShadow {
                    transparentBorder: true
                    radius: 2
                    samples: 2
                    horizontalOffset: 0
                    verticalOffset: 0
                    color: Qt.rgba(0, 0, 0, 0.11)
                }
            }
        }
    }

    background: Rectangle {
        color: LingmoUI.Theme.alternateBackgroundColor
        radius: LingmoUI.Theme.smallRadius + 2
    }
}
