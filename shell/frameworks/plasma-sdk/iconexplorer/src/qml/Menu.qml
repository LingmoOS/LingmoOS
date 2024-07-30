/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.8 as Kirigami

Item {
    Loader {
        id:menuLoader
        sourceComponent: actionsMenu
        onLoaded: {
            item.actions = cuttlefish.actions
        }
         Connections {
            target: cuttlefish
            function onItemRightClicked() {
                menuLoader.item.popup()
            }
        }
    }
    Component {
        id: actionsMenu
        QQC2.Menu {
            id: theMenu
            property Component submenuComponent: actionsMenu
            property Component itemDelegate: QQC2.MenuItem { }
            property Component seperatorDelegate: Kirigami.Separator {}
            property alias actions: actionsInstantiator.model
            Item {
                id: invisibleItems
                visible: false
            }
            Instantiator {
                id: actionsInstantiator
                active: theMenu.visible
                delegate: QtObject {
                    readonly property var action: modelData
                    property QtObject item: null
                    function create() {
                        if (!action.hasOwnProperty("children") && !action.children || action.children.length === 0) {
                            item = theMenu.itemDelegate.createObject(null, { action: action });
                            theMenu.addItem(item)
                        } else {
                            item = theMenu.submenuComponent.createObject(null, { actions: action.children, title: action.text});
                            theMenu.insertMenu(theMenu.count, item)
                            var menuitem = theMenu.contentData[theMenu.contentData.length-1]
                            menuitem.icon = action.icon
                        }
                    }
                    function remove() {
                        if (!action.hasOwnProperty("children") && !action.children || action.children.length === 0) {
                            theMenu.removeItem(item)
                        } else if (theMenu.submenuComponent) {
                            theMenu.removeMenu(item)
                        }
                    }
                }
                onObjectAdded: object.create()
                onObjectRemoved: object.remove()
            }
        }
    }
}
