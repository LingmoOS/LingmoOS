// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQml.Models 2.1
import org.deepin.dtk 1.0 as D

Repeater {
    property string filterText

    model: D.ObjectModelProxy {
        filterAcceptsItem: function(item) {
            return item.text.includes(filterText)
        }
        sourceModel: ObjectModel {
            Text { text: "Page1"}
            Text { text: "Page2"}
            Text { text: "Page3"}
        }
    }
}
