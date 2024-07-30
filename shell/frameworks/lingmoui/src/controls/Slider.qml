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

import QtQuick 2.6
import QtQuick.Templates 2.3 as T
import Qt5Compat.GraphicalEffects
import org.kde.lingmoui as LingmoUI

T.Slider {
    id: control

    property int sliderTrackHeight: 5

    implicitWidth: background.implicitWidth
    implicitHeight: 22
    snapMode: T.Slider.SnapOnRelease

    handle: Rectangle {
        id: handleRect
        x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))

        width: control.implicitHeight
        height: width
        radius: width / 2
        opacity: 1
        antialiasing: true
        border.width: 0

        color: control.pressed ? "#FAFAFA" : "white"

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            radius: 8
            samples: 12
            horizontalOffset: 0
            verticalOffset: 0
            color: Qt.rgba(0, 0, 0, control.pressed ? 0.25 : 0.15)
        }
    }

    background: Item {
        implicitWidth: control.horizontal ? 200 : control.implicitHeight
        implicitHeight: control.horizontal ? control.implicitHeight : 200

        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        width: control.horizontal ? control.availableWidth : implicitWidth
        height: control.horizontal ? implicitHeight : control.availableHeight
        scale: control.horizontal && control.mirrored ? -1 : 1

        Rectangle {
            x: control.horizontal ? 0 : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : 0
            width: control.horizontal ? parent.width : sliderTrackHeight
            height: !control.horizontal ? parent.height : sliderTrackHeight
            radius: !control.horizontal ? parent.width / 2 : sliderTrackHeight / 2
            color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.5) : Qt.rgba(0, 0, 0, 0.1)
        }

        Rectangle {
            x: control.horizontal ? 0 : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : control.visualPosition * parent.height
            width: control.horizontal ? control.position * parent.width : sliderTrackHeight
            height: !control.horizontal ? control.position * parent.height : sliderTrackHeight
            radius: !control.horizontal ? parent.width / 2 : sliderTrackHeight / 2
            color: control.LingmoUI.Theme.highlightColor
        }
    }
}
