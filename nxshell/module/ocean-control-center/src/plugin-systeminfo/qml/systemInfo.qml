// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    name: "auxiliaryInfo"
    parentName: "system"
    displayName: qsTr("Auxiliary Information")
    weight: 1000
    pageType: OceanUIObject.Item
    page: Label {
        font: OceanUIUtils.copyFont(D.DTK.fontManager.t4, {
                                    "bold": true
                                })
        text: oceanuiObj.displayName
    }
    onParentItemChanged: {
        if (parentItem) {
            parentItem.topPadding = 10
            parentItem.leftPadding = 10
        }
    }
}
