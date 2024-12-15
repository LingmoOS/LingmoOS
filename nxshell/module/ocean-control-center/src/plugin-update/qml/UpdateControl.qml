// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import Qt.labs.qmlmodels 1.2
import QtQuick.Layouts 1.15
import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "updateCtl"
        parentName: "checkUpdate"
        backgroundType: OceanUIObject.AutoBg
        weight: 10
        visible: oceanuiData.model().showUpdateCtl
        pageType: OceanUIObject.Item

        page: RowLayout {
            ColumnLayout {
                RowLayout {
                    spacing: 0
                    OceanUICheckIcon {
                        visible: oceanuiData.model().updateState === "success"
                        checked: true
                        size: 18
                    }

                    D.Label {
                        Layout.leftMargin: 10
                        font.pixelSize: 18
                        font.bold: true
                        text: oceanuiData.model().updateStateTips
                    }
                }

                D.Label {
                    visible: false
                    text: qsTr("预计安装时间：30min")
                    font.pixelSize: 12
                }
            }

            D.Button {
                Layout.alignment: Qt.AlignRight
                text: oceanuiData.model().actionBtnText
                font.pixelSize: 14
                implicitWidth: 120
                visible: oceanuiData.model().updateState !== "upgrading"
                onClicked: {
                    oceanuiData.work().onActionBtnClicked();
                }
            }

            D.BusyIndicator {
                id: scanAnimation

                Layout.alignment: Qt.AlignRight
                running: oceanuiData.model().updateState === "upgrading"
                visible: oceanuiData.model().updateState === "upgrading"
                implicitWidth: 32
                implicitHeight: 32
            }

            ColumnLayout {
                visible: false
                Layout.alignment: Qt.AlignRight

                RowLayout {
                    spacing: 10
                    D.ProgressBar {
                        id: process

                        Layout.alignment: Qt.AlignHCenter
                        from: 0
                        to: 100
                        value: 45
                        implicitHeight: 8
                        implicitWidth: 240
                    }

                    D.DciIcon {
                        name: "qrc:/icons/lingmo/builtin/icons/update_stop.png"
                        width: 16
                        height: 16
                    }

                    D.DciIcon {
                        name: "qrc:/icons/lingmo/builtin/icons/update_close.png"
                        width: 16
                        height: 16
                    }
                }

                Label {
                    text: qsTr("Installing") + process.value + "%"
                    font.pixelSize: 12
                }
            }
        }
    }

    OceanUIObject {
        name: "updateList"
        parentName: "checkUpdate"
        backgroundType: OceanUIObject.Normal
        weight: 20
        visible: oceanuiData.model().showUpdateCtl
        pageType: OceanUIObject.Item

        page: UpdateList {
            model: ListModel {
                ListElement {
                    name: qsTr("Feature Updates")
                    checked: true
                }
            }
        }
    }
}
