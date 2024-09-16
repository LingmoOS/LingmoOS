// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 2.15

Item {
    id: root
    required property bool useColumnLayout
    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias spacing: listView.spacing
    property alias count: listView.count
    property alias add: listView.add
    property alias remove: listView.remove
    property alias move: listView.move
    property alias displaced: listView.displaced
    ListView {
        id: listView
        anchors.fill: parent
        orientation: useColumnLayout ? ListView.Vertical : ListView.Horizontal
        layoutDirection: Qt.LeftToRight
        verticalLayoutDirection: ListView.TopToBottom
        interactive: false
    }

    function calculateImplicitWidth(prev, current) {
        if (useColumnLayout) {
            return Math.max(prev, current)
        } else {
            if (prev == 0) {
                return current
            }
            return prev + root.spacing + current
        }
    }

    function calculateImplicitHeight(prev, current) {
        if (useColumnLayout) {
            if (prev == 0) {
                return current
            }
            return prev + root.spacing + current
        } else {
            return Math.max(prev, current)
        }
    }

    implicitWidth: {
        let width = 0
        for (let child of listView.contentItem.visibleChildren) {
            width = calculateImplicitWidth(width, child.implicitWidth)
        }
        return width
    }
    implicitHeight: {
        let height = 0
        for (let child of listView.contentItem.visibleChildren) {
            height = calculateImplicitHeight(height, child.implicitHeight)
        }
        return height
    }
}
