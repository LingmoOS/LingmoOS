/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    width: 600
    height: 600

    LingmoUI.Theme.inherit: false
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

    header: Controls.MenuBar {
        Controls.Menu {
            title: "&File"

            Controls.MenuItem {
                checkable: true
                text: "Item1"
                icon.name: "go-next"
                icon.color: "red"
            }
            Controls.MenuItem {
                text: "Item2"
                icon.name: "folder-video"
            }
            Controls.MenuSeparator {
            }
            Controls.MenuItem {
                checkable: true
                text: "Item3"
            }
            Controls.Menu {
                title: "Submenu"

                Controls.MenuItem {
                    checkable: true
                    text: "Item1"
                    icon.name: "go-next"
                    icon.color: "red"
                }
                Controls.MenuItem {
                    text: "Item2"
                    icon.name: "folder-video"
                }
                Controls.MenuItem {
                    text: "Item3"
                }
            }
        }
        Controls.Menu {
            title: "Edit"

            Controls.MenuItem {
                text: "Item1"
                icon.name: "go-next"
            }
            Controls.MenuItem {
                text: "Item2"
                icon.name: "go-next"
            }
        }
    }

    Rectangle {
        id: background

        anchors.centerIn: parent
        width: childrenRect.width
        height: childrenRect.height

        color: LingmoUI.Theme.backgroundColor
        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

        ColumnLayout {
            spacing: LingmoUI.Units.smallSpacing

            Controls.ComboBox {
                LingmoUI.Theme.inherit: true
                currentIndex: 1
                model: ["View", "Window", "Button", "Selection", "Tooltip", "Complementary"]
                onCurrentTextChanged: background.LingmoUI.Theme.colorSet = currentText
            }
            Controls.Button {
                LingmoUI.Theme.inherit: true
                text: "text"
                icon.name: "go-previous"
            }
            Controls.Button {
                id: coloredIconButton
                text: "text"
                icon.name: "go-previous"
            }
            RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                Controls.Label {
                    text: "RGB color for icon:"
                }
                Controls.SpinBox {
                    id: red
                    editable: true
                    from: 0
                    to: 255
                    onValueModified: {
                        coloredIconButton.icon.color = Qt.rgba(red.value/255, green.value/255, blue.value/255, 1);
                    }
                }
                Controls.SpinBox {
                    id: green
                    LingmoUI.Theme.inherit: true
                    editable: true
                    from: 0
                    to: 255
                    onValueModified: {
                        coloredIconButton.icon.color = Qt.rgba(red.value/255, green.value/255, blue.value/255, 1);
                    }
                }
                Controls.SpinBox {
                    id: blue
                    editable: true
                    from: 0
                    to: 255
                    onValueModified: {
                        coloredIconButton.icon.color = Qt.rgba(red.value/255, green.value/255, blue.value/255, 1);
                    }
                }
            }
            Controls.ToolButton {
                text: "text"
                icon.name: "go-previous"
            }
        }
    }
}
