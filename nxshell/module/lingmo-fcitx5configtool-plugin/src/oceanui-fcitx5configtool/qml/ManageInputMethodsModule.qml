// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    // title
    OceanUIObject {
        name: "ManageInputMethodsTitle"
        parentName: "Manage Input Methods"
        displayName: qsTr("Input method management")
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

            D.Button {
                id: addImBtn
                text: qsTr("Add input method")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }
                InputMethodsChooser {
                    id: imsChooser
                    viewModel: oceanuiData.imProxyModel()
                    onSelected: function (index) {
                        console.log("selected im index", index)
                        oceanuiData.addIM(index)
                    }
                }

                onClicked: {
                    console.log("Add input method button clicked")
                    imsChooser.active = true
                }
            }
        }
    }

    // list
    OceanUIObject {
        name: "currentIMList"
        parentName: "Manage Input Methods"
        weight: 120
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: IMList {}
    }
}
