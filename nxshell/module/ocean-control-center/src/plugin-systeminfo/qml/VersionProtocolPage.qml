// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0
import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        id: versionProtocolPage
        name: "title"
        parentName: "system/versionProtocol"
        pageType: OceanUIObject.Item
        weight: 20
        page: Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                font: DTK.fontManager.t4
                text: oceanuiData.systemInfoMode().gnuLinceseTitle
            }
    }
    OceanUIObject {
        id: versionProtocolPageContent
        name: "content"
        parentName: "system/versionProtocol"
        pageType: OceanUIObject.Item
        weight: 30
        page: Label {
            Layout.alignment: Qt.AlignVCenter
            font: DTK.fontManager.t6
            horizontalAlignment: Text.AlignLeft
            text: oceanuiData.systemInfoMode().gnuLinceseContent
            wrapMode: Text.WordWrap
            opacity: 0.7
        }
    }
}
