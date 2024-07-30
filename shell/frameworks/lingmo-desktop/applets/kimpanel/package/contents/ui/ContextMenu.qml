/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

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

    function open(item, actionItem) {
        visualParent = item;
        refreshMenu(actionItem);
        menu.openRelative();
    }

    function refreshMenu(actionItem) {
        if (menu) {
            menu.destroy();
        }

        menu = contextMenuComponent.createObject(root, {"actionItem": actionItem});

        if (actionList && actionList.length > 0) {
            menu.separator.visible = true;
            actionList.forEach(function(actionItem) {
                var item = contextMenuItemComponent.createObject(menu.showMenu, {
                    "actionItem": actionItem,
                });
            });
        }
    }

    Component {
        id: contextMenuItemComponent

        LingmoExtras.MenuItem {
            property variant actionItem

            text: actionItem.label
            icon: actionItem.icon

            onClicked: {
                kimpanel.showAction(actionItem.key);
            }
        }
    }

    Component {
        id: contextMenuComponent

        LingmoExtras.Menu {
            visualParent: root.visualParent
            property variant actionItem
            property Item separator: separatorItem
            property QtObject showMenu: subShowMenu

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

            LingmoExtras.MenuItem {
                id: showItem
                visible: separatorItem.visible
                text: i18n("Show")

                property LingmoExtras.Menu _subShowMenu: LingmoExtras.Menu {
                    id: subShowMenu
                    visualParent: showItem.action
                }
            }

            LingmoExtras.MenuItem {
                id: separatorItem
                separator: true
                visible: false
            }

            LingmoExtras.MenuItem {
                text: i18n("Hide %1", actionItem.label);
                onClicked: {
                    kimpanel.hideAction(actionItem.key);
                }
                enabled: actionItem.key !== 'kimpanel-placeholder'
                visible: enabled
            }

            LingmoExtras.MenuItem {
                text: i18n("Configure Input Method")
                icon: "configure"
                onClicked: {
                    helper.configure();
                }
            }

            LingmoExtras.MenuItem {
                text: i18n("Reload Config")
                icon: "view-refresh"
                onClicked: {
                    helper.reloadConfig();
                }
            }

            LingmoExtras.MenuItem {
                text: i18n("Exit Input Method")
                icon: "application-exit"
                onClicked: {
                    helper.exit();
                }
            }

            LingmoExtras.MenuItem {
                property QtObject configureAction: null

                enabled: configureAction && configureAction.enabled

                text: configureAction ? configureAction.text : ""
                icon: configureAction ? configureAction.icon : ""

                onClicked: configureAction.trigger()

                Component.onCompleted: configureAction = Plasmoid.internalAction("configure")
            }
        }
    }
}
