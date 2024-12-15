// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import "./Utils"

RowLayout {
    anchors.fill: parent
    spacing: Utils.spacing
    Category {
        id: category
        Layout.fillHeight: true
        Layout.preferredWidth: Utils.listItemWidth
        Layout.leftMargin: Utils.margin
        onCurrentItemChanged: {
            content.source = category.model.get(category.currentIndex).source
        }
    }
    Rectangle {
        clip: true
        Layout.fillHeight: true
        Layout.fillWidth: true
        radius: Utils.listRadius
        color: Utils.transparentBackground
        Loader {
            id: content
            anchors.fill: parent
        }
    }
}

