// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.4

Item {
    id: control
    property alias source: image.source
    property real imageScale: 1
    signal clicked
    default property alias content: loader.sourceComponent
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    function trigger() {
        if (loader.item) {
            if (loader.item.hasOwnProperty("popup")) {
                loader.item.popup(control)
            } else if (loader.item.hasOwnProperty("open")) {
                loader.item.open()
            } else if (loader.item.hasOwnProperty("show")) {
                loader.item.show()
            }
        }
        control.clicked()
    }

    Image {
        id: image
        width: sourceSize.width * imageScale
        height: sourceSize.height * imageScale
        MouseArea {
            anchors.fill: parent
            onClicked: trigger()
        }
        Loader {
            id: loader
        }
    }
}
