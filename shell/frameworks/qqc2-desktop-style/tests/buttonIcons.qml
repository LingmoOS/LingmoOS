/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    width: 600
    height: 600

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

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

        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.ComboBox {
                Kirigami.Theme.inherit: true
                currentIndex: 1
                model: ["View", "Window", "Button", "Selection", "Tooltip", "Complementary"]
                onCurrentTextChanged: background.Kirigami.Theme.colorSet = currentText
            }
            Controls.Button {
                Kirigami.Theme.inherit: true
                text: "text"
                icon.name: "go-previous"
            }
            Controls.Button {
                id: coloredIconButton
                text: "text"
                icon.name: "go-previous"
            }
            RowLayout {
                spacing: Kirigami.Units.smallSpacing

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
                    Kirigami.Theme.inherit: true
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
