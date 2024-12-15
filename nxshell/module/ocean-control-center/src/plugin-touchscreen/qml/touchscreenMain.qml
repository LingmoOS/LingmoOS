// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "touchscreen"
        parentName: "touchscreen"
        displayName: qsTr("Common")
        weight: 10
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 5
            isGroup: false
            height: implicitHeight + 20
        }
        TouchScreen {}
    }
}
