// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

D.SortFilterModel {
    id: model

    property int leftDockOrder: 0
    property int rightDockOrder: 0

    model: Applet.appletItems

    filterAcceptsItem: function(item) {
        return item.data.dockOrder > leftDockOrder && item.data.dockOrder <= rightDockOrder && (item.data.shouldVisible === undefined || item.data.shouldVisible)
    }
    lessThan: function(leftItem, rightItem) {
        return parseInt(leftItem.data.dockOrder) <= parseInt(rightItem.data.dockOrder)
    }
    delegate: Control {
        contentItem: model.data
        Component.onCompleted: {
            contentItem.parent = this
        }
    }
}
