/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.plasmoid 2.0

Item {
    id: root

    property QtObject menu
    property Item visualParent
    property variant actionList

    signal actionClicked(string actionId)

    onActionListChanged: refreshMenu();

    function open() {
        menu.openRelative();
    }

    function refreshMenu() {
        if (menu) {
            menu.destroy();
        }

        menu = contextMenuComponent.createObject(root);

        if (!actionList || actionList.length === 0) {
            var item = emptyMenuItemComponent.createObject(menu);

            menu.addMenuItem(item);

            return;
        }

        actionList.forEach(function(actionItem) {
            var item = contextMenuItemComponent.createObject(menu, {
                "actionItem": actionItem,
            });

            menu.addMenuItem(item);
        });
    }

    Component {
        id: contextMenuComponent

        LingmoExtras.Menu {
            visualParent: root.visualParent

            placement: {
                if (Plasmoid.location === LingmoCore.Types.LeftEdge) {
                    return LingmoExtras.Menu.RightPosedTopAlignedPopup;
                } else if (Plasmoid.location === LingmoCore.Types.TopEdge) {
                    return LingmoExtras.Menu.BottomPosedLeftAlignedPopup;
                } else if (Plasmoid.location === LingmoCore.Types.RightEdge) {
                    return LingmoExtras.Menu.LeftPosedTopAlignedPopup;
                } else {
                    return LingmoExtras.Menu.TopPosedLeftAlignedPopup;
                }
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        LingmoExtras.MenuItem {
            property variant actionItem

            text: actionItem.text ? actionItem.text : ""
            icon: actionItem.icon ? actionItem.icon : null
            checkable: actionItem.hint === "checked"
            checked: actionItem.hint === "checked"

            onClicked: {
                actionClicked(actionItem.actionId);
            }
        }
    }

    Component {
        id: emptyMenuItemComponent
        LingmoExtras.MenuItem {
            text: i18n("(Empty)")
            enabled: false
        }
    }
}
