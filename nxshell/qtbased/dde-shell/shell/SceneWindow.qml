// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.ds 1.0

ApplicationWindow {
    id: root
    visible: true
    width: view.implicitWidth
    height: view.implicitHeight

    Control {
        id: view
        padding: 10
        anchors.fill: parent

        implicitWidth: listView.contentWidth + leftPadding + rightPadding
        implicitHeight: listView.contentHeight + topPadding + bottomPadding

        contentItem: ListView {
            id: listView
            spacing: 10
            contentWidth: {
                let value = 0
                for (let child of listView.contentItem.visibleChildren) {
                    value = Math.max(value, child.width)
                }
                return value
            }

            model: DS.rootApplet.appletItems
            delegate: Control {
                contentItem: model.data
                Component.onCompleted: {
                    contentItem.parent = this
                }
            }
        }
    }
}
