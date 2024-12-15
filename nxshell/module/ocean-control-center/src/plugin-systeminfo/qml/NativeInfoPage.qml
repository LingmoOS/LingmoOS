// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// import org.lingmo.dtk 1.0 as D
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.dtk 1.0
import org.lingmo.oceanui 1.0
OceanUIObject {
    id: root11
    OceanUIObject {
        name: "systemLogo"
        weight: 10
        parentName: "systemInfo"
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal
        visible: !oceanuiData.systemInfoMode().showDetail
        page: RowLayout {
            Image {
                // source: "qrc:/icons/lingmo/builtin/icons/oceanui_nav_systeminfo_84px.svg"
                source: "file://" + DTK.lingmoDistributionOrgLogo
            }

            ColumnLayout {
                Label {
                    font.bold: true
                    font.pixelSize: 22
                    horizontalAlignment: Text.AlignLeft
                    text: qsTr("UOS")
                }

                Label {
                    font: DTK.fontManager.t6
                    horizontalAlignment: Text.AlignLeft
                    text: oceanuiData.systemInfoMode().systemCopyright
                }
            }
        }
    }

    OceanUIObject {
        name: "systemDetailLogo"
        weight: 20
        parentName: "systemInfo"
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal
        visible: oceanuiData.systemInfoMode().showDetail
        page: ColumnLayout{
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Image {
                Layout.topMargin: 10
                Layout.alignment: Qt.AlignHCenter
                source: "file://" + oceanuiData.systemInfoMode().logoPath
            }
            Label {
                Layout.alignment: Qt.AlignHCenter
                font: DTK.fontManager.t6
                text: oceanuiData.systemInfoMode().systemCopyright
                Layout.bottomMargin: 10
            }
        }
    }

    OceanUIObject {
        name: "nativeInfoGrp"
        parentName: "systemInfo"
        weight: 40
        pageType: OceanUIObject.Item
        page: ColumnLayout {
            OceanUIGroupView {
                Layout.topMargin: 10
            }
        }
        OceanUIObject {
            name: "productName"
            weight: 10
            parentName: "nativeInfoGrp"
            displayName: qsTr("Computer name") + ":"
            backgroundType: OceanUIObject.Normal
            visible: oceanuiData.systemInfoMode().showDetail
            pageType: OceanUIObject.Editor
            page: RowLayout {

                Label {
                    id: hostNameLabel
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    text: oceanuiData.systemInfoMode().hostName
                    font: DTK.fontManager.t6
                }

                IconButton {
                    id: editBtn
                    icon.name: "qrc:/icons/lingmo/builtin/texts/oceanui_edit_12px.svg"
                    icon.width: 12
                    icon.height: 12
                    implicitWidth: 20
                    implicitHeight: 20
                    background: Rectangle {
                        color: "transparent" // 背景透明
                        border.color: "transparent" // 边框透明
                        border.width: 0
                    }
                    onClicked: {
                        editBtn.visible = false
                        hostNameLabel.visible = false
                        hostNameEdit.visible = true
                        hostNameEdit.text = oceanuiData.systemInfoMode().hostName
                        hostNameEdit.selectAll()
                    }
                }

                LineEdit {
                    id: hostNameEdit
                    horizontalAlignment: Text.AlignHCenter
                    text: oceanuiData.systemInfoMode().hostName
                    visible: false
                    showAlert: false
                    validator: RegularExpressionValidator {
                        regularExpression: /^[A-Za-z0-9-]{0,64}$/ // 允许的输入模式
                    }
                    onEditingFinished: {

                        if (hostNameEdit.text.length === 0) {
                            editBtn.visible = true
                            hostNameLabel.visible = true
                            hostNameEdit.visible = false
                            hostNameLabel.showAlert = false
                            return
                        }

                        if ((hostNameEdit.text.indexOf('-') === 0 || hostNameEdit.text.lastIndexOf('-') === hostNameEdit.text.length - 1) && hostNameEdit.text.length <= 63) {

                            hostNameEdit.showAlert = true
                            hostNameEdit.alertText = qsTr("It cannot start or end with dashes")
                            return
                        }

                        editBtn.visible = true
                        hostNameLabel.visible = true
                        hostNameEdit.visible = false
                        hostNameEdit.showAlert = false
                        oceanuiData.systemInfoWork().onSetHostname(hostNameEdit.text)

                    }
                    Keys.onPressed: {
                        if (event.key === Qt.Key_Return) {
                            hostNameEdit.forceActiveFocus(false); // 结束编辑
                        }
                    }
                    onFocusChanged: {
                        console.log(" =============== hostNameEdit Focus " )
                    }
                }
            }
        }
        OceanUIObject {
            name: "hostName"
            weight: 20
            parentName: "nativeInfoGrp"
            displayName: qsTr("OS Name") + ":"
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Label {
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                text: oceanuiData.systemInfoMode().productName
                font: DTK.fontManager.t6
            }
        }
        OceanUIObject {
            name: "version"
            weight: 30
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Version") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().versionNumber
            }
        }
        OceanUIObject {
            name: "edition"
            weight: 40
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Edition") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().version
            }
        }
        OceanUIObject {
            name: "type"
            weight: 50
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Type") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().type+ "-" + qsTr("bit")
            }
        }

        OceanUIObject {
            name: "authorization"
            weight: 60
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Authorization") + ":"
            visible: oceanuiData.systemInfoMode().showAuthorization()
            page: RowLayout {
                Label {
                    color: oceanuiData.systemInfoMode().licenseStatusColor
                    font: DTK.fontManager.t6
                    horizontalAlignment: Text.AlignLeft
                    text: oceanuiData.systemInfoMode().licenseStatusText
                }

                ToolButton {
                    text: oceanuiData.systemInfoMode().licenseActionText
                    ColorSelector.family: Palette.CommonColor
                    font: DTK.fontManager.t6
                    flat: false
                    visible: oceanuiData.systemInfoMode().showDetail
                    opacity: 0.7
                    onClicked: {
                        oceanuiData.systemInfoWork().showActivatorDialog()
                    }
                }
            }
        }

        OceanUIObject {
            name: "systemInstallationTime"
            weight: 70
            visible: oceanuiData.systemInfoMode().showAuthorization()
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("System installation time") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().systemInstallationDate
            }
        }

        OceanUIObject {
            name: "kernel"
            weight: 80
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Kernel") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().kernel
            }
        }

        OceanUIObject {
            name: "graphicsPlatform"
            weight: 90
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            visible: oceanuiData.systemInfoMode().showGraphicsPlatform()
            displayName: qsTr("Graphics Platform") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().graphicsPlatform
            }
        }

        OceanUIObject {
            name: "processor"
            weight: 100
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Processor") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().processor
            }
        }

        OceanUIObject {
            name: "memory"
            weight: 100
            parentName: "nativeInfoGrp"
            pageType: OceanUIObject.Editor
            displayName: qsTr("Memory") + ":"
            page: Label {
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignLeft
                text: oceanuiData.systemInfoMode().memory
            }
        }
    }

    OceanUIObject {
        name: "detailBtn"
        weight: 60
        parentName: "systemInfo"
        pageType: OceanUIObject.Item
        visible: !oceanuiData.systemInfoMode().showDetail
        page: RowLayout {

            Button {
                Layout.topMargin: 10
                implicitWidth: 250
                implicitHeight: 30
                Layout.alignment: Qt.AlignHCenter
                text: "显示详细信息"
                font: DTK.fontManager.t6
                opacity: 0.7
                onClicked: {
                    oceanuiData.systemInfoWork().showDetail()
                }
            }
        }
    }
}
