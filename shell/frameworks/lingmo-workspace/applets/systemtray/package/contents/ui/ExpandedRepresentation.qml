/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

Item {
    id: popup

    Layout.minimumWidth: LingmoUI.Units.gridUnit * 24
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 24

    property alias hiddenLayout: hiddenItemsView.layout
    property alias plasmoidContainer: container

    // Header
    LingmoExtras.PlasmoidHeading {
        id: plasmoidHeading
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: trayHeading.height + bottomPadding + container.headingHeight
        Behavior on height {
            NumberAnimation { duration: LingmoUI.Units.shortDuration / 2; easing.type: Easing.InOutQuad }
        }
    }

    // Main content layout
    ColumnLayout {
        id: expandedRepresentation
        anchors.fill: parent
        // TODO: remove this so the scrollview fully touches the header;
        // add top padding internally
        spacing: plasmoidHeading.bottomPadding

        // Header content layout
        RowLayout {
            id: trayHeading
            Layout.fillWidth: true

            LingmoComponents.ToolButton {
                id: backButton
                visible: systemTrayState.activeApplet && systemTrayState.activeApplet.expanded && (hiddenLayout.itemCount > 0)
                icon.name: mirrored ? "go-previous-symbolic-rtl" : "go-previous-symbolic"

                display: LingmoComponents.AbstractButton.IconOnly
                text: i18nc("@action:button", "Go Back")

                KeyNavigation.down: hiddenItemsView.visible ? hiddenLayout : container

                onClicked: systemTrayState.setActiveApplet(null)
            }

            LingmoUI.Heading {
                Layout.fillWidth: true
                leftPadding: systemTrayState.activeApplet ? 0 : LingmoUI.Units.largeSpacing

                level: 1
                text: systemTrayState.activeApplet ? systemTrayState.activeApplet.plasmoid.title : i18n("Status and Notifications")
                textFormat: Text.PlainText
                elide: Text.ElideRight
            }

            Repeater {
                id: primaryActionButtons

                model: {
                    if (actionsButton.applet === null) {
                        return [];
                    }
                    return actionsButton.applet.plasmoid.contextualActions.filter(action => {
                        return !action.isSeparator && action.priority === LingmoCore.Action.HighPriority;
                    });
                }

                delegate: LingmoComponents.ToolButton {
                    // We cannot use `action` as it is already a QQuickAction property of the button
                    property QtObject qAction: model.modelData

                    visible: qAction && qAction.visible

                    // NOTE: it needs an IconItem because QtQuickControls2 buttons cannot load QIcons as their icon
                    contentItem: LingmoUI.Icon {
                        anchors.centerIn: parent
                        active: parent.hovered
                        implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                        implicitHeight: implicitWidth
                        source: parent.qAction ? parent.qAction.icon.name : ""
                    }

                    enabled: qAction && qAction.enabled
                    checkable: qAction && qAction.checkable
                    checked: qAction && qAction.checked
                    display: LingmoComponents.AbstractButton.IconOnly
                    text: qAction ? qAction.text : ""

                    KeyNavigation.down: backButton.KeyNavigation.down
                    KeyNavigation.left: (index > 0) ? primaryActionButtons.itemAt(index - 1) : backButton
                    KeyNavigation.right: (index < primaryActionButtons.count - 1) ? primaryActionButtons.itemAt(index + 1) :
                                                            actionsButton.visible ? actionsButton : actionsButton.KeyNavigation.right

                    LingmoComponents.ToolTip {
                        text: parent.text
                    }

                    onClicked: {
                        if (qAction) {
                            qAction.trigger();
                        }
                    }
                }
            }

            LingmoComponents.ToolButton {
                id: actionsButton
                visible: visibleActions > 0
                enabled: visibleActions > 1 || (singleAction && singleAction.enabled)
                checked: visibleActions > 1 ? configMenu.status !== LingmoExtras.Menu.Closed : singleAction && singleAction.checked
                property QtObject applet: systemTrayState.activeApplet || root
                property int visibleActions: menuItemFactory.count
                property QtObject singleAction: visibleActions === 1 && menuItemFactory.object ? menuItemFactory.object.action : null

                icon.name: "application-menu"
                checkable: visibleActions > 1 || (singleAction && singleAction.checkable)
                contentItem.opacity: visibleActions > 1

                display: LingmoComponents.AbstractButton.IconOnly
                text: actionsButton.singleAction ? actionsButton.singleAction.text : i18n("More actions")

                Accessible.role: actionsButton.singleAction ? Accessible.Button : Accessible.ButtonMenu

                KeyNavigation.down: backButton.KeyNavigation.down
                KeyNavigation.right: configureButton.visible ? configureButton : configureButton.KeyNavigation.right

                // NOTE: it needs a LingmoUI.Icon because QtQuickControls2 buttons cannot load QIcons as their icon
                LingmoUI.Icon {
                    parent: actionsButton
                    anchors.centerIn: parent
                    active: actionsButton.hovered
                    implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                    implicitHeight: implicitWidth
                    source: actionsButton.singleAction !== null ? actionsButton.singleAction.icon.name : ""
                    visible: actionsButton.singleAction
                }
                onToggled: {
                    if (visibleActions > 1) {
                        if (checked) {
                            configMenu.openRelative();
                        } else {
                            configMenu.close();
                        }
                    }
                }
                onClicked: {
                    if (singleAction) {
                        singleAction.trigger();
                    }
                }
                LingmoComponents.ToolTip {
                    text: parent.text
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
                        if (!actionsButton.applet) {
                            return [];
                        }
                        return actionsButton.applet.plasmoid.contextualActions
                            .filter(action => {
                                return action.visible &&
                                    action.priority === LingmoCore.Action.NormalPriority &&
                                    action !== actionsButton.applet.plasmoid.internalAction("configure");
                            })
                            // squash separators
                            .reduce((dstActions, action, i, srcActions) => {
                                if (!action.isSeparator) {
                                    const prevAction = srcActions[i - 1];
                                    if (prevAction?.isSeparator && dstActions.length > 0) {
                                        dstActions.push(prevAction);
                                    }
                                    dstActions.push(action);
                                }
                                return dstActions;
                            }, []);
                    }
                    delegate: LingmoExtras.MenuItem {
                        id: menuItem
                        action: modelData
                    }
                    onObjectAdded: (index, object) => {
                        configMenu.addMenuItem(object);
                    }
                }
            }
            LingmoComponents.ToolButton {
                id: configureButton
                icon.name: "configure"
                visible: actionsButton.applet && actionsButton.applet.plasmoid.internalAction("configure")

                display: LingmoComponents.AbstractButton.IconOnly
                text: actionsButton.applet.plasmoid.internalAction("configure") ? actionsButton.applet.plasmoid.internalAction("configure").text : ""

                KeyNavigation.down: backButton.KeyNavigation.down
                KeyNavigation.left: actionsButton.visible ? actionsButton : actionsButton.KeyNavigation.left
                KeyNavigation.right: pinButton

                LingmoComponents.ToolTip {
                    text: parent.visible ? parent.text : ""
                }
                onClicked: actionsButton.applet.plasmoid.internalAction("configure").trigger();
            }

            LingmoComponents.ToolButton {
                id: pinButton
                checkable: true
                checked: Plasmoid.configuration.pin
                onToggled: Plasmoid.configuration.pin = checked
                icon.name: "window-pin"

                display: LingmoComponents.AbstractButton.IconOnly
                text: i18n("Keep Open")

                KeyNavigation.down: backButton.KeyNavigation.down
                KeyNavigation.left: configureButton.visible ? configureButton : configureButton.KeyNavigation.left

                LingmoComponents.ToolTip {
                    text: parent.text
                }
            }
        }

        // Grid view of all available items
        HiddenItemsView {
            id: hiddenItemsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: LingmoUI.Units.smallSpacing
            visible: !systemTrayState.activeApplet

            KeyNavigation.up: pinButton

            onVisibleChanged: {
                if (visible) {
                    layout.forceActiveFocus();
                    systemTrayState.oldVisualIndex = systemTrayState.newVisualIndex = -1;
                }
            }
        }

        // Container for currently visible item
        PlasmoidPopupsContainer {
            id: container
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: systemTrayState.activeApplet

            // We need to add margin on the top so it matches the dialog's own margin
            Layout.topMargin: mergeHeadings ? 0 : dialog.topPadding

            KeyNavigation.up: pinButton
            KeyNavigation.backtab: pinButton

            onVisibleChanged: {
                if (visible) {
                    forceActiveFocus();
                }
            }
        }
    }

    // Footer
    LingmoExtras.PlasmoidHeading {
        id: plasmoidFooter
        position: LingmoComponents.ToolBar.Footer
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        visible: container.appletHasFooter
        height: container.footerHeight
        // So that it doesn't appear over the content view, which results in
        // the footer controls being inaccessible
        z: -9999
    }
}
