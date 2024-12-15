//SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "pluginAreaTitle"
        weight: 10
        parentName: "personalization/dock/pluginArea"
        pageType: OceanUIObject.Item
        displayName: qsTr("Plugin Area")
        description: qsTr("Select which icons appear in the Dock")
        page: ColumnLayout {
            Label {
                font.family: D.DTK.fontManager.t4.family
                font.bold: true
                font.pixelSize: D.DTK.fontManager.t4.pixelSize
                text: oceanuiObj.displayName
            }
            Label {
                text: oceanuiObj.description
            }
        }
    }

    OceanUIObject {
        name: "pluginAreaView"
        parentName: "personalization/dock/pluginArea"
        weight: 100
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIRepeater {
            model: oceanuiData.pluginModel
            delegate: OceanUIObject {
                name: "plugin" + model.key
                property real iconSize: 16
                parentName: "personalization/dock/pluginArea/pluginAreaView"
                weight: 10 + index * 10
                backgroundType: OceanUIObject.Normal
                icon: model.icon
                displayName: model.displayName
                pageType: OceanUIObject.Editor
                page: OceanUICheckIcon {
                    checked: model.visible
                    onClicked: {
                        oceanuiData.dockInter.setItemOnDock(model.settingKey, model.key, !checked)
                    }
                }
            }
        }
    }
}
