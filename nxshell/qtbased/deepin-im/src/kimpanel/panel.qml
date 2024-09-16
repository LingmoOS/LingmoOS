// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml.Models 2.15
import Qt.labs.platform 1.1

Window {
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    x: kimpanel.pos.x
    y: kimpanel.pos.y
    width: container.width
    height: container.height
    visible: kimpanel.showAux || kimpanel.showPreedit || kimpanel.showLookupTable
    title: qsTr("DIM panel")

    Column {
        id: container

        Row {
            Row {
                visible: kimpanel.showAux

                Text {
                    text: kimpanel.aux.text
                }
                Text {
                    text: kimpanel.aux.attr
                }
            }

            Row {
                visible: kimpanel.showPreedit

                Text {
                    text: kimpanel.preedit.text.slice(0, kimpanel.preeditCaretPos) + "|" + kimpanel.preedit.text.slice(kimpanel.preeditCaretPos)
                }
                Text {
                    text: kimpanel.preedit.attr
                }
            }
        }

        Row {
            Repeater{
                visible: kimpanel.showLookupTable

                model: kimpanel.lookupTable
                delegate: Row {
                    Text {
                        id: label
                        text: modelData.label
                    }
                    Text {
                        id: text
                        text: modelData.text
                    }
                    Text {
                        id: attr
                        text: modelData.attr
                    }
                }
            }
        }
    }

    SystemTrayIcon {
        visible: kimpanel.properties.length != 0
        icon.name: kimpanel.properties.length != 0 ? kimpanel.properties[0].icon : ""

        menu: Menu {
            id: contextMenu

            Instantiator {
                model: kimpanel.properties

                delegate: MenuItem {
                    text: modelData.label
                    icon.name: modelData.icon
                    onTriggered: kimpanel.menuTriggered(modelData.key)
                }

                onObjectAdded: function(index, object) {
                    contextMenu.insertItem(index, object)
                }
                onObjectRemoved: function(index, object) {
                    contextMenu.removeItem(object)
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("Reload Configuration")
                onTriggered: kimpanel.reloadTriggered()
            }

            MenuItem {
                text: qsTr("Settings")
                onTriggered: kimpanel.configureTriggered()
            }
        }
    }
}
