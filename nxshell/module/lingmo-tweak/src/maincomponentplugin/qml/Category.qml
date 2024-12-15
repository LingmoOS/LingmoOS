// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.15
import "."
import "./Utils"
import "./Components"

ListView {
    id: listView
    model: CategoryModel {}
    Layout.topMargin: Utils.margin
    spacing: Utils.spacing
    focus: true
    interactive: false
    highlight: ListViewHighLight {}
    delegate: ListViewDelegate {
        width: listView.width
        height: Utils.listItemHeight
        isCurrentIndex: listView.currentIndex === index
        onClicked: {
            listView.currentIndex = index
        }
    }
}

