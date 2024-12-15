// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "AdvancedSettingsTitle"
        parentName: "Manage Input Methods"
        displayName: qsTr("Advanced Settings")
        weight: 310
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
            Label {
                leftPadding: 10
                rightPadding: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t8
                text: qsTr(
                          "\"Advanced Settings\" is only valid for the input method that uses "
                          + "the system settings, if the input method has its own settings, "
                          + "its own settings shall prevail.")
                opacity: 0.5
                wrapMode: Text.Wrap
            }
        }
    }

    OceanUIObject {
        name: "GlobalConfigPage"
        parentName: "Manage Input Methods"
        displayName: qsTr("Global Config")
        weight: 320
        page: OceanUIRightView {
            spacing: 5
        }
        GlobalConfigPage {}
    }

    OceanUIObject {
        name: "AddonsPage"
        parentName: "Manage Input Methods"
        displayName: qsTr("Add-ons")
        weight: 330
        page: OceanUIRightView {
            spacing: 5
        }
        AddonsPage {}
    }
}
