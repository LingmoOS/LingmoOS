// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0
import org.lingmo.oceanui 1.0
//import org.lingmo.oceanui.systemInfo 1.0

OceanUIObject {

    OceanUIObject {
        name: "title"
        parentName: "system/privacyPolicy"
        pageType: OceanUIObject.Item
        weight: 20
        page: Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            font: DTK.fontManager.t4
            text: qsTr("Privacy Policy")
        }
    }
    OceanUIObject {
        name: "content"
        parentName: "system/privacyPolicy"
        pageType: OceanUIObject.Item
        weight: 30
        page:
            Label {
                textFormat: Text.RichText
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().privacyPolicy
                wrapMode: Text.WordWrap
                opacity: 0.7

                // 超链接点击事件
                onLinkActivated: function(url) {
                    console.log("点击的链接是: " + url)
                    Qt.openUrlExternally(url) // 使用默认浏览器打开链接
                }
            }
    }
}
