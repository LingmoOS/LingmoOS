/*
    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: root
    property QtObject effectFrame: null
    x: 0
    y: 0
    width: effectFrame ? effectFrame.size.width : 0
    height: effectFrame ? effectFrame.size.height : 0
    color: "transparent"
    Rectangle {
        id: rect2;
        width: parent.width;
        height: parent.height;
        border.color: root.effectFrame ? root.effectFrame.color : "";
        border.width: 4;
        color: "transparent";
        radius: root.effectFrame ? root.effectFrame.radius : 0.0;
    }
}
