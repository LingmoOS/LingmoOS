// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15

Item {
    id: control
    property list<Item> items

    signal aboutToGrabToImage(Item item)
    signal grabToImageFinished(Item item)

    onItemsChanged: function updateGeomotry() {
        var w = 0
        var h = 0
        for (var i = 0; i < control.items.length; ++i) {
            let item = control.items[i]
            w = Math.max(w, item.width)
            h = Math.max(h, item.height)
        }
        control.width = w + 30
        control.height = h * 2 + 30
    }

    function addItem(item) {
        if (control.items.include(item))
            return

        items.push(item)
    }

    function removeItem(item) {
        if (!control.items.include(item))
            return

        var index = control.items.indexOf(item);
        if (index !== -1) {
          control.items.splice(index, 1);
        }
    }

    Repeater {
        model: control.items.length
        delegate: Image {
            id: img
            anchors.centerIn: parent
            antialiasing: true
            rotation: index ? (index % 2 === 0 ? 10 : -10) : 0
            opacity: (1 - index * 0.2)
            z: -index

            Component.onCompleted: {
                let item = control.items[index]
                if (!item)
                    return

                img.width = item.width
                img.height = item.height

                aboutToGrabToImage(item)
                item.grabToImage(function(result) {
                    img.source = result.url
                    grabToImageFinished(item)
                })
            }
        }
    }

    RoundButton {
        id: number
        checked: true
        anchors.right: dragItem.right
        anchors.top: dragItem.top
        anchors.margins: 8
        implicitWidth: 24
        implicitHeight: 24
        text: control.items.length > 1  ? control.items.length : ""
        opacity: control.items.length > 1 ? 1 : 0
    }
}
