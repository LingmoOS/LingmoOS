/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.RangeSlider {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            first.implicitHandleWidth + leftPadding + rightPadding,
                            second.implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             first.implicitHandleHeight + topPadding + bottomPadding,
                             second.implicitHandleHeight + topPadding + bottomPadding)

    padding: 6

    first.handle: Rectangle {
        property bool horizontal: control.orientation === Qt.Horizontal
        x: control.leftPadding + (horizontal ? control.first.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (horizontal ? (control.availableHeight - height) / 2 : control.first.visualPosition * (control.availableHeight - height))
        implicitWidth: 18
        implicitHeight: 18
        radius: width / 2
        border.color: control.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        color: Kirigami.Theme.backgroundColor
        Rectangle {
            z: -1
            x: 1
            y: 1
            width: parent.width
            height: parent.height
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.15)
        }
    }

    second.handle: Rectangle {
        property bool horizontal: control.orientation === Qt.Horizontal
        x: control.leftPadding + (horizontal ? control.second.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (horizontal ? (control.availableHeight - height) / 2 : control.second.visualPosition * (control.availableHeight - height))
        implicitWidth: 18
        implicitHeight: 18
        radius: width / 2
        border.color: control.activeFocus ? Kirigami.Theme.highlightColor :  Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        color: Kirigami.Theme.backgroundColor
        Rectangle {
            z: -1
            x: 1
            y: 1
            width: parent.width
            height: parent.height
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.15)
        }
    }

    background: Rectangle {
        id: backgroundRectangle

        readonly property bool horizontal: control.orientation === Qt.Horizontal
        implicitWidth: horizontal ? 200 : 6
        implicitHeight: horizontal ? 6 : 200
        width: horizontal ? control.availableWidth : implicitWidth
        height: horizontal ? implicitHeight : control.availableHeight
        radius: Math.round(Math.min(width / 2, height / 2))
        color: Qt.alpha(Kirigami.Theme.textColor, 0.3)
        anchors.centerIn: parent

        Rectangle {
            x: backgroundRectangle.horizontal
                ? (LayoutMirroring.enabled
                   ? parent.width - width - control.first.position * parent.width
                   : control.first.position * parent.width)
                : 0
            y: backgroundRectangle.horizontal ? 0 : control.second.visualPosition * parent.height + 6
            width: backgroundRectangle.horizontal ? control.second.position * parent.width - control.first.position * parent.width - 6 : 6
            height: backgroundRectangle.horizontal ? 6 : control.second.position * parent.height - control.first.position * parent.height - 6
            color: Kirigami.Theme.highlightColor
        }
    }
}
