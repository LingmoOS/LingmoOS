/*
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.qqc2desktopstyle.private as StylePrivate

Item {
    id: handle

    required property T.Slider control

    readonly property StylePrivate.StyleItem styleItem: {
        const item = control.background;
        return (item instanceof StylePrivate.StyleItem) ? item : null;
    }

    // It won't keep track of an actual position, but QtQuick.Templates code
    // only accounts for handle size and does not care for x/y anyway.
    property size size

    function updateHandleSize() {
        if (styleItem) {
            const rect = styleItem.subControlRect("handle");
            size = Qt.size(rect.width, rect.height);
        }
    }

    x: control.leftPadding + Math.round(control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
    y: control.topPadding + Math.round(control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))

    implicitWidth: size.width
    implicitHeight: size.height

    Connections {
        target: handle.styleItem

        function onStyleNameChanged() {
            handle.updateHandleSize();
        }
    }

    Component.onCompleted: {
        updateHandleSize();
    }
}
