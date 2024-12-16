// Copyright (C) 2023 pengwenhao <pengwenhao@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.lingmo.dtk 1.0

Control {
    id: control
    property string title
    property int corners: RoundRectangle.NoneCorner
    property Component action
    padding: 5
    contentItem: RowLayout {
            Label {
                visible: control.title
                Layout.fillWidth: true
                text: control.title
                font: DTK.fontManager.t8
                elide: Text.ElideMiddle
            }
            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
    }

    background: RoundRectangle {
        implicitWidth: 66
        implicitHeight: 40
        color:  Qt.rgba(0, 0, 0, 0.05)
        radius: Style.control.radius
        corners: control.corners
    }
}
