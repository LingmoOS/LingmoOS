// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3
import QtQuick.Window 2.15
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    property var configOptions: []
    property var keyName: ""

    OceanUIObject {
        name: root.displayName
        parentName: "GlobalConfigPage"
        weight: 40
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            height: implicitHeight + 30
        }
    }

    OceanUIObject {
        id: headerItem
        property bool expanded: false
        parentName: root.displayName
        displayName: root.displayName
        weight: root.weight
        pageType: OceanUIObject.Item
        backgroundType: OceanUIObject.Normal | OceanUIObject.Hover
        page: RowLayout {
            id: headerRow

            MouseArea {
                anchors.fill: parent
                onClicked: headerItem.expanded = !headerItem.expanded
            }

            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
            D.ToolButton {
                Layout.alignment: Qt.AlignRight
                icon.width: 12
                icon.height: 12
                icon.name: headerItem.expanded ? "go-down" : "go-next"
                onClicked: headerItem.expanded = !headerItem.expanded
            }
        }
    }

    Component.onCompleted: {
        oceanuiData.fcitx5ConfigProxy.onRequestConfigFinished.connect(() => {
                                                                      configOptions = []
                                                                      configOptions = oceanuiData.fcitx5ConfigProxy.globalConfigOptions(
                                                                          root.name)
                                                                      keyName = ""
                                                                  })
        configOptions = oceanuiData.fcitx5ConfigProxy.globalConfigOptions(root.name)
        loading = false
    }

    Loader {
        active: !loading
        asynchronous: true
        sourceComponent: OceanUIRepeater {
            model: configOptions
            delegate: Component {
                OceanUIObject {
                    parentName: root.displayName
                    displayName: modelData.description
                    weight: root.weight + index + 1
                    pageType: OceanUIObject.Editor
                    visible: headerItem.expanded
                    backgroundType: OceanUIObject.Normal | OceanUIObject.Hover
                    page: Loader {
                        sourceComponent: {
                            switch (modelData.type) {
                            case "Boolean":
                                return booleanComponent
                            case "Integer":
                                return integerComponent
                            case "String":
                                return stringComponent
                            case "List|Key":
                                return keyComponent
                            case "Enum":
                                return enumComponent
                            default:
                                return null
                            }
                        }

                        Component {
                            id: booleanComponent
                            D.Switch {
                                checked: modelData.value === "True"
                                onCheckedChanged: {
                                    oceanuiData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                checked ? "True" : "False")
                                }
                            }
                        }

                        Component {
                            id: integerComponent
                            D.SpinBox {
                                width: 55
                                implicitWidth: 55
                                value: parseInt(modelData.value)
                                onValueChanged: {
                                    oceanuiData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                value.toString())
                                }
                            }
                        }

                        Component {
                            id: stringComponent
                            D.TextField {
                                text: modelData.value
                                onTextChanged: {
                                    oceanuiData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                text)
                                }
                            }
                        }

                        Component {
                            id: keyComponent
                            KeySequenceDisplay {
                                placeholderText: qsTr("Please enter a new shortcut")
                                keys: modelData.value
                                background.visible: false
                                onFocusChanged: {
                                    if (!focus) {
                                        if (keys.length > 0) {
                                            oceanuiData.fcitx5ConfigProxy.setValue(
                                                        root.name + "/" + modelData.name + "/0",
                                                        keys, true)
                                        } else if (keyName != modelData.name) {
                                            keys = modelData.value
                                        }
                                    }
                                }
                                onKeysChanged: {
                                    root.keyName = modelData.name
                                }

                                Connections {
                                    target: root
                                    function onKeyNameChanged() {
                                        if (keyName != modelData.name) {
                                            focus = false
                                        }
                                    }
                                }
                            }
                        }

                        Component {
                            id: enumComponent
                            D.ComboBox {
                                model: modelData.propertiesI18n
                                flat: true
                                currentIndex: modelData.properties.indexOf(
                                                  modelData.value) ? modelData.properties.indexOf(
                                                                         modelData.value) : 0
                                onCurrentIndexChanged: {
                                    oceanuiData.fcitx5ConfigProxy.setValue(
                                                root.name + "/" + modelData.name,
                                                modelData.properties[currentIndex])
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
