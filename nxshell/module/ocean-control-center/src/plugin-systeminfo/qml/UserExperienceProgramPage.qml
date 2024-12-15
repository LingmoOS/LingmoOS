// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    name: "userExperienceProgramGrp"
    parentName: "system/userExperienceProgram"
    pageType: OceanUIObject.Item
    weight: 20
    page: OceanUIGroupView {}
    OceanUIObject {
        name: "userExperienceProgramSwitch"
        weight: 20
        parentName: "system/userExperienceProgram/userExperienceProgramGrp"
        displayName: qsTr("Join User Experience Program")
        pageType: OceanUIObject.Editor
        page: ColumnLayout {
            D.Switch {
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                checked: oceanuiData.systemInfoMode().joinUeProgram
                onCheckedChanged: {
                    console.log("userExperienceProgramSwitch clicked ")
                    oceanuiData.systemInfoWork().setUeProgram(checked)
                }
            }

            Window {
                id: modalDialog
                visible: false
                flags: Qt.Window
                modality: Qt.ApplicationModal
                color: "transparent"
                D.DWindow.enabled: true
                opacity: 0.0
            }

            Connections {
                target: oceanuiData
                function onRequestUeProgram(visible) {

                    if (visible) {
                        modalDialog.show()
                    } else {
                        modalDialog.close()
                    }
                }
            }

        }
    }
    OceanUIObject {
        name: "userExperienceProgramContent"
        weight: 30
        parentName: "system/userExperienceProgram/userExperienceProgramGrp"
        pageType: OceanUIObject.Item
        page: D.Label {
            anchors.fill: parent
            anchors.margins: 10
            font: DTK.fontManager.t6
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: oceanuiData.systemInfoMode().userExperienceProgramText

            // 超链接点击事件
            onLinkActivated: function(url) {
                console.log("点击的链接是: " + url)
                Qt.openUrlExternally(url) // 使用默认浏览器打开链接
            }
        }
    }
}
