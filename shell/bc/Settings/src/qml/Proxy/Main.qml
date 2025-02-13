/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQml 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Proxy")

    Binding {
        target: ftpProxyTextField
        property: "text"
        value: httpProxyField.text
        when: forFtpCheckBox.checked
        restoreMode: Binding.RestoreValue
    }

    NetworkProxy {
        id: networkProxy
    }

    Binding {
        target: ftpProxyPortField
        property: "text"
        value: httpProxyPortField.text
        when: forFtpCheckBox.checked
        restoreMode: Binding.RestoreNone
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing * 2

            RoundedItem {
                id: mainItem
                spacing: LingmoUI.Units.largeSpacing

                RadioButton {
                    id: noProxyRadioButton
                    checked: networkProxy.flag === 0
                    text: qsTr("No Proxy")
                    onClicked: {
                        networkProxy.flag = 0
                    }
                }

                RadioButton {
                    id: autoScriptProxyRadioButton
                    checked: networkProxy.flag === 1
                    text: qsTr("Use proxy auto configuration URL")
                    visible: false
                    onClicked: {
                        networkProxy.flag = 1
                    }
                }

                RadioButton {
                    id: manualProxyRadioButton
                    checked: networkProxy.flag === 2
                    text: qsTr("Use manually specified proxy configuration")
                    onClicked: {
                        networkProxy.flag = 2
                    }
                }
            }

            RoundedItem {
                id: proxyItem
                visible: !noProxyRadioButton.checked

                property bool needSave: false

                Component.onCompleted: needSave = false

                RowLayout {
                    id: autoScriptProxyLayout
                    spacing: LingmoUI.Units.largeSpacing
                    visible: autoScriptProxyRadioButton.checked

                    TextField {
                        id: autoScriptField
                        Layout.fillWidth: true
                        text: networkProxy.scriptProxy
                        height: 40

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Button {
                        text: qsTr("Select file")
                        flat: true
                        onClicked: fileDialog.open()
                    }
                }

                // ----------------------
                GridLayout {
                    visible: manualProxyRadioButton.checked
                    columns: 4
                    columnSpacing: LingmoUI.Units.largeSpacing
                    rowSpacing: LingmoUI.Units.largeSpacing

                    Label {
                        text: qsTr("HTTP Proxy")
                    }

                    TextField {
                        id: httpProxyField
                        Layout.fillWidth: true
                        height: 40
                        text: networkProxy.httpProxy
                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        id: httpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80

                        text: networkProxy.httpProxyPort

                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Item {
                        width: 1
                    }

                    CheckBox {
                        id: forFtpCheckBox
                        text: qsTr("Also use this proxy for FTP")
                        Layout.fillWidth: true
                        checked: networkProxy.useSameProxy
                        onClicked: {
                            proxyItem.needSave = true
                        }
                    }

                    Item {
                        width: 1
                    }

                    Item {
                        width: 1
                    }

                    // FTP
                    Label {
                        text: qsTr("FTP Proxy")
                    }

                    TextField {
                        id: ftpProxyTextField
                        Layout.fillWidth: true
                        height: 40
                        enabled: !forFtpCheckBox.checked
                        text: networkProxy.ftpProxy
                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        id: ftpProxyPortField
                        height: 40
                        Layout.preferredWidth: 80
                        enabled: !forFtpCheckBox.checked

                        text: networkProxy.ftpProxyPort
                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    // SOCKS
                    Label {
                        text: qsTr("SOCKS Proxy")
                    }

                    TextField {
                        id: socksProxyField
                        Layout.fillWidth: true
                        height: 40
                        text: networkProxy.socksProxy

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }

                    Label {
                        text: qsTr("Port")
                    }

                    TextField {
                        id: socksProxyPortField
                        height: 40
                        Layout.preferredWidth: 80

                        text: networkProxy.socksProxyPort
                        validator: IntValidator { bottom: 0; top: 999999}

                        onTextChanged: {
                            proxyItem.needSave = true
                        }
                    }
                }

                HorizontalDivider {
                    visible: proxyItem.needSave
                }

                RowLayout {
                    visible: proxyItem.needSave

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Save")

                        onClicked: {
                            proxyItem.needSave = false
                            networkProxy.scriptProxy = autoScriptField.text
                            networkProxy.httpProxy = httpProxyField.text
                            networkProxy.httpProxyPort = httpProxyPortField.text
                            networkProxy.useSameProxy = forFtpCheckBox.checked
                            networkProxy.ftpProxy = ftpProxyTextField.text
                            networkProxy.ftpProxyPort = ftpProxyPortField.text
                            networkProxy.socksProxy = socksProxyField.text
                            networkProxy.socksProxyPort = socksProxyPortField.text
                        }
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }
        }
    }

    FileDialog {
        id: fileDialog
        onAccepted: {
            autoScriptField.text = fileDialog.fileUrl.toString().replace("file://", "")
            networkProxy.scriptProxy = autoScriptField.text
        }
    }
}
