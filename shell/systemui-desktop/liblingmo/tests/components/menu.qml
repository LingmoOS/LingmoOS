/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.components as PC3
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Extras Menu"
    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.gridUnit

        PC3.Button {
            text: "Simple menu"
            onClicked: simpleMenu.open(0, height)

            LingmoExtras.Menu {
                id: simpleMenu

                LingmoExtras.MenuItem { text: "Hello" }
                LingmoExtras.MenuItem { text: "This is just a simple" }
                LingmoExtras.MenuItem { text: "Menu" }
                LingmoExtras.MenuItem { text: "without separators" }
                LingmoExtras.MenuItem { text: "and other stuff" }
            }
        }

        PC3.Button {
            text: "Checkable menu items"
            onClicked: checkableMenu.open(0, height)

            LingmoExtras.Menu {
                id: checkableMenu

                LingmoExtras.MenuItem { text: "Apple"; checkable: true }
                LingmoExtras.MenuItem { text: "Banana"; checkable: true }
                LingmoExtras.MenuItem { text: "Orange"; checkable: true }
            }
        }


        PC3.Button {
            text: "Icons"
            onClicked: iconsMenu.open(0, height)

            LingmoExtras.Menu {
                id: iconsMenu

                LingmoExtras.MenuItem { text: "Error"; icon: "dialog-error" }
                LingmoExtras.MenuItem { text: "Warning"; icon: "dialog-warning" }
                LingmoExtras.MenuItem { text: "Information"; icon: "dialog-information" }
            }
        }

        PC3.Button {
            text: "Separators and sections"
            onClicked: sectionsMenu.open(0, height)

            LingmoExtras.Menu {
                id: sectionsMenu

                LingmoExtras.MenuItem { text: "A menu"; section: true }
                LingmoExtras.MenuItem { text: "One entry" }
                LingmoExtras.MenuItem { text: "Another entry" }
                LingmoExtras.MenuItem { separator: true }
                LingmoExtras.MenuItem { text: "One item" }
                LingmoExtras.MenuItem { text: "Another item" }
            }
        }

        RowLayout {
            spacing: LingmoUI.Units.smallSpacing

            PC3.Button {
                id: minMaxButton
                text: "Fixed minimum and maximum width"
                onClicked: minMaxMenu.open(0, height)

                LingmoExtras.Menu {
                    id: minMaxMenu

                    minimumWidth: minMaxButton.width
                    maximumWidth: limitMenuMaxWidth.checked ? minMaxButton.width : undefined // has a RESET property

                    LingmoExtras.MenuItem { text: "Hello" }
                    LingmoExtras.MenuItem { text: "This is just a simple" }
                    LingmoExtras.MenuItem { text: "Menu" }
                    LingmoExtras.MenuItem { text: "with some very very long text in one item that will "
                                                    + "make the menu super huge if you don't do anything about it" }
                    LingmoExtras.MenuItem { text: "and other stuff" }
                }
            }

            PC3.CheckBox {
                id: limitMenuMaxWidth
                anchors.verticalCenter: parent.verticalCenter
                text: "Limit maximum width"
                checked: true
            }
        }

        PC3.Button {
            text: "Don't crash on null MenuItem action"
            onClicked: noActionCrashMenu.open(0, height)

            LingmoExtras.Menu {
                id: noActionCrashMenu

                LingmoExtras.MenuItem { text: "This is an item" }
                LingmoExtras.MenuItem { text: "Below me should NOT be an empty item"}
                LingmoExtras.MenuItem { action: null }
                LingmoExtras.MenuItem { text: "I am not empty" }
            }
        }
    }
}
