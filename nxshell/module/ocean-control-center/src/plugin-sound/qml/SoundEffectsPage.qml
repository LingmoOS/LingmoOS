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
        name: "soundEffects"
        parentName: "sound/soundEffectsPage"
        displayName: qsTr("Sound Effects")
        weight: 10
        pageType: OceanUIObject.Editor
        page: Switch {
            checked: oceanuiData.model().enableSoundEffect
            onCheckedChanged: {
                oceanuiData.worker().enableAllSoundEffect(checked)
            }
        }
    }
    OceanUIObject {
        name: "effectsList"
        parentName: "sound/soundEffectsPage"
        weight: 20
        visible: oceanuiData.model().enableSoundEffect
        pageType: OceanUIObject.Item
        page: DeviceListView {
            backgroundVisible: false
            showPlayBtn: true
            model: oceanuiData.model().soundEffectsModel()
            onClicked: function (index, checked) {
                oceanuiData.worker().setSoundEffectEnable(index, checked)
            }

            onPlaybtnClicked: function (index) {
                console.log(" effectsList click play ", index)

                oceanuiData.worker().playSoundEffect(index)
            }
        }
    }
}
