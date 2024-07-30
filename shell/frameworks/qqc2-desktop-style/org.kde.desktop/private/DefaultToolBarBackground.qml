/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    required property T.ToolBar control

    implicitHeight: 40
    color: Kirigami.Theme.backgroundColor

    Kirigami.Separator {
        id: separator
        anchors {
            left: parent.left
            right: parent.right
        }
    }

    // Conditional anchors are not reliable, and state machine are chunky in
    // terms of number of objects.
    function __fixup() {
        // Make sure to unset an old anchor before assigning a new one,
        // or else the separator will stuck being stretched vertically.
        if (control?.position === T.ToolBar.Header) {
            separator.anchors.top = undefined;
            separator.anchors.bottom = root.bottom;
        } else {
            separator.anchors.bottom = undefined;
            separator.anchors.top = root.top;
        }
    }

    Component.onCompleted: __fixup()

    Connections {
        target: root.control

        function onPositionChanged() {
            root.__fixup();
        }
    }
}
