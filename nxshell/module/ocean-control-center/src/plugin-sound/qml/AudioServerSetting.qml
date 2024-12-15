// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0
import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "audioServerTitle"
        parentName: "sound/audioServerSetting"
        displayName: qsTr("Audio Framework")
        description: qsTr("Different audio frameworks have their own advantages and disadvantages, and you can choose the one that best matches you to use")
        weight: 10
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.AutoBg
        page: ColumnLayout {
            spacing: 2
            Label {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 15
                font.pixelSize: 16
                font.bold: true
                text: oceanuiObj.displayName
            }
            Label {
                Layout.alignment: Qt.AlignLeft
                font.pixelSize: 12
                Layout.leftMargin: 15
                text: oceanuiObj.description
            }
        }
    }
    OceanUIObject {
        name: "audioServerGrp"
        parentName: "sound/audioServerSetting"
        weight: 20
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal

        page: DeviceListView {
            enabled: oceanuiData.model().audioServerStatus
            backgroundVisible: true
            showPlayBtn: false
            model: oceanuiData.model().audioServerModel()
            onClicked: function (index, checked) {
                if (checked) {
                    oceanuiData.worker().setAudioServerIndex(index)
                }
            }
        }
    }
}
