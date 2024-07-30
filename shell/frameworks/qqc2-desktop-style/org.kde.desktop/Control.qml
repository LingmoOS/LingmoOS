/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.qqc2desktopstyle.private as StylePrivate

T.Control {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: styleItem.pixelMetric("layouttopmargin")
    leftPadding: styleItem.pixelMetric("layoutleftmargin")
    rightPadding: styleItem.pixelMetric("layoutrightmargin")
    bottomPadding: styleItem.pixelMetric("layoutbottommargin")

    property Item __style: StylePrivate.StyleItem {
        id: styleItem
    }
}
