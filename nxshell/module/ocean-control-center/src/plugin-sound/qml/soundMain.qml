// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "outPut"
        parentName: "sound"
        displayName: qsTr("Output")
        icon: "system"
        weight: 10
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 5
            isGroup: false
        }
        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 20
            }
        }
        SpeakerPage {}
    }
    OceanUIObject {
        name: "inPut"
        parentName: "sound"
        displayName: qsTr("Input")
        icon: "system"
        weight: 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 5
            isGroup: false
        }
        onParentItemChanged: {
            if (parentItem) {
                parentItem.bottomPadding = 20
            }
        }
        MicrophonePage {}
    }
    OceanUIObject {
        name: "soundEffectsPage"
        parentName: "sound"
        displayName: qsTr("Sound Effects")
        description: qsTr("Enable/disable sound effects")
        icon: "system_sound"
        weight: 30
        page: OceanUIRightView {
            isGroup: true
        }
        SoundEffectsPage {}
    }
    OceanUIObject {
        name: "deviceManger"
        parentName: "sound"
        displayName: qsTr("DevicesManger")
        description: qsTr("Enable/disable audio devices")
        icon: "equipment_management"
        visible: oceanuiData.model().inPutPortCount !== 0 || oceanuiData.model().outPutCount !== 0
        weight: 40
        SoundDevicemanagesPage {}
    }
    OceanUIObject {
        name: "audioServerSetting"
        parentName: "sound"
        displayName: qsTr("Audio Framework")
        description: qsTr("Choose the appropriate audio framework")
        icon: "audio_framework"
        weight: 50
        AudioServerSetting {}
    }
}
