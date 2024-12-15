// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.2

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.fcitx5configtool 1.0

OceanUIObject {

    // title
    OceanUIObject {
        name: "AddonsListTitle"
        parentName: "AddonsPage"
        displayName: qsTr("Add-ons")
        weight: 110
        pageType: OceanUIObject.Item
        page: RowLayout {
            width: parent.width
            Label {
                Layout.leftMargin: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    DetailAddonsItem {}
}

