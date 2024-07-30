/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

 /**
  * A standard basic header for plasmoids which has title, a config button and
  * a popup menu with all extra plasmoid actions.
  *
  * By default, it will be invisible when the plasmoid is in the system tray,
  * as it provides a replacement header with the same features
  *
  * @inherit PlasmoidHeading
  */
PlasmoidHeading {
    /**
     * extraControls: list<QtObject>
     *
     * Any extra control and button that may be inserted in the heading
     */
    default property alias extraControls: extraControlsLayout.data

    visible: !(Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading)

    contentItem: RowLayout {
        Heading {
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            Layout.fillWidth: true
            level: 1
            text: Plasmoid.title
        }
        RowLayout {
            id: extraControlsLayout
            visible: children.length > 0
            Layout.fillHeight: true
        }
        LingmoComponents.ToolButton {
            id: actionsButton
            visible: visibleActions > 0
            checked: configMenu.status !== LingmoExtras.Menu.Closed
            property int visibleActions: menuItemFactory.count
            property QtObject singleAction: visibleActions === 1 ? menuItemFactory.object.action : null
            icon.name: "open-menu-symbolic"
            checkable: visibleActions > 1
            contentItem.opacity: visibleActions > 1
            // NOTE: it needs an Icon because QtQuickControls2 buttons cannot load QIcons as their icon
            LingmoUI.Icon {
                parent: actionsButton
                anchors.centerIn: parent
                active: actionsButton.hovered
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                implicitHeight: implicitWidth
                source: actionsButton.singleAction !== null ? actionsButton.singleAction.icon : ""
                visible: actionsButton.singleAction
            }
            onToggled: {
                if (checked) {
                    configMenu.openRelative();
                } else {
                    configMenu.close();
                }
            }
            onClicked: {
                if (singleAction) {
                    singleAction.trigger();
                }
            }
            LingmoComponents.ToolTip {
                text: actionsButton.singleAction ? actionsButton.singleAction.text : i18nd("liblingmo6", "More actions")
            }
            LingmoExtras.Menu {
                id: configMenu
                visualParent: actionsButton
                placement: LingmoExtras.Menu.BottomPosedLeftAlignedPopup
            }

            Instantiator {
                id: menuItemFactory
                model: {
                    configMenu.clearMenuItems();
                    const configureAction = Plasmoid.internalAction("configure");
                    const actions = Plasmoid.contextualActions
                        .filter(action => action !== configureAction);
                    return actions;
                }
                delegate: LingmoExtras.MenuItem {
                    required property QtObject modelData // type: QAction
                    action: modelData
                }
                onObjectAdded: {
                    configMenu.addMenuItem(object);
                }
            }
        }
        LingmoComponents.ToolButton {
            id: configureButton

            property LingmoCore.Action internalAction

            function fetchInternalAction() {
                internalAction = Plasmoid.internalAction("configure");
            }

            Connections {
                target: Plasmoid
                function onInternalActionsChanged(actions) {
                    configureButton.fetchInternalAction();
                }
            }

            Component.onCompleted: fetchInternalAction()

            icon.name: "configure"
            visible: internalAction !== null
            text: internalAction?.text ?? ""
            display: T.AbstractButton.IconOnly
            LingmoComponents.ToolTip {
                text: configureButton.text
            }
            onClicked: internalAction?.trigger();
        }
    }
}
