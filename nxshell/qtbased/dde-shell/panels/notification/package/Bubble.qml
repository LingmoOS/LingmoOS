// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

D.Control {
    id: control
    property var bubble

    contentItem:  Loader {
        sourceComponent: bubble.level <= 1 ? normalCom : overlayCom
    }
    Component {
        id: normalCom
        NormalBubble {
            bubble: control.bubble
        }
    }
    Component {
        id: overlayCom
        OverlayBubble {
            bubble: control.bubble
        }
    }

    z: bubble.level <= 1 ? 0 : 1 - bubble.level
}
