/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: root

    name: "OverlayZStackingTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    // Layers are not set
    Component {
        id: defaultComponent
        T.Popup {
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: defaultDrawerComponent
        T.Drawer {
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: defaultDialogComponent
        T.Dialog {
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: defaultMenuComponent
        T.Menu {
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: defaultToolTipComponent
        T.ToolTip {
            z: LingmoUI.OverlayZStacking.z
        }
    }

    function test_defaultLayers() {
        let popup = null;

        popup = createTemporaryObject(defaultComponent, this);
        verify(popup);
        compare(popup.LingmoUI.OverlayZStacking.layer, LingmoUI.OverlayZStacking.DefaultLowest);
        compare(popup.z, 0);

        popup = createTemporaryObject(defaultDrawerComponent, this);
        compare(popup.LingmoUI.OverlayZStacking.layer, LingmoUI.OverlayZStacking.Drawer);
        verify(popup);
        compare(popup.z, 100);

        popup = createTemporaryObject(defaultDialogComponent, this);
        verify(popup);
        compare(popup.LingmoUI.OverlayZStacking.layer, LingmoUI.OverlayZStacking.Dialog);
        compare(popup.z, 300);

        popup = createTemporaryObject(defaultMenuComponent, this);
        verify(popup);
        compare(popup.LingmoUI.OverlayZStacking.layer, LingmoUI.OverlayZStacking.Menu);
        compare(popup.z, 400);

        popup = createTemporaryObject(defaultToolTipComponent, this);
        verify(popup);
        compare(popup.LingmoUI.OverlayZStacking.layer, LingmoUI.OverlayZStacking.ToolTip);
        compare(popup.z, 600);
    }

    // Layers are set
    Component {
        id: drawerComponent
        T.Drawer {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.Drawer
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: fullScreenComponent
        T.Popup {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.FullScreen
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: dialogComponent
        T.Dialog {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.Dialog
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: menuComponent
        T.Menu {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.Menu
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: notificationComponent
        T.ToolTip {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.Notification
            z: LingmoUI.OverlayZStacking.z
        }
    }
    Component {
        id: toolTipComponent
        T.ToolTip {
            LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.ToolTip
            z: LingmoUI.OverlayZStacking.z
        }
    }

    function createWithLayers() {
        const drawer = createTemporaryObject(drawerComponent, this);
        verify(drawer);
        const fullScreen = createTemporaryObject(fullScreenComponent, this);
        verify(fullScreen);
        const dialog = createTemporaryObject(dialogComponent, this);
        verify(dialog);
        const menu = createTemporaryObject(menuComponent, this);
        verify(menu);
        const notification = createTemporaryObject(notificationComponent, this);
        verify(notification);
        const toolTip = createTemporaryObject(toolTipComponent, this);
        verify(toolTip);
        return ({
            drawer,
            fullScreen,
            dialog,
            menu,
            notification,
            toolTip,
        });
    }

    function test_stackingNaturalOrder() {
        const all = createWithLayers();
        const { drawer, fullScreen, dialog, menu, notification, toolTip } = all;

        drawer      .parent = this;
        fullScreen  .parent = drawer       .contentItem;
        dialog      .parent = fullScreen   .contentItem;
        menu        .parent = dialog       .contentItem;
        notification.parent = menu         .contentItem;
        toolTip     .parent = notification .contentItem;

        drawer.open();
        compare(drawer.z, 100);
        fullScreen.open();
        compare(fullScreen.z, 200);
        dialog.open();
        compare(dialog.z, 300);
        menu.open();
        compare(menu.z, 400);
        notification.open();
        compare(notification.z, 500);
        toolTip.open();
        compare(toolTip.z, 600);
    }

    function test_stackingReverseOrder() {
        const all = createWithLayers();
        const { drawer, fullScreen, dialog, menu, notification, toolTip } = all;

        toolTip     .parent = this;
        notification.parent = toolTip     .contentItem;
        menu        .parent = notification.contentItem;
        dialog      .parent = menu        .contentItem;
        fullScreen  .parent = dialog      .contentItem;
        drawer      .parent = fullScreen  .contentItem;

        toolTip.open();
        compare(toolTip.z, 600);
        notification.open();
        compare(notification.z, 601);
        menu.open();
        compare(menu.z, 602);
        dialog.open();
        compare(dialog.z, 603);
        fullScreen.open();
        compare(fullScreen.z, 604);
        drawer.open();
        compare(drawer.z, 605);
    }

    Component {
        id: spyComponent
        SignalSpy {}
    }

    function test_parentChangesZ() {
        const parent = createTemporaryObject(defaultComponent, this, { parent: this });
        const child = createTemporaryObject(defaultComponent, this);
        const spy = createTemporaryObject(spyComponent, this, {
            target: child,
            signalName: "zChanged",
        });
        verify(spy.valid);
        compare(spy.count, 0);
        compare(parent.z, 0);

        child.parent = parent.contentItem;
        compare(spy.count, 1);
        compare(child.z, 1);

        parent.z = 42;
        compare(spy.count, 2);
        compare(child.z, 43);

        spy.clear();
        parent.open();
        child.open();
        // deferred signal
        parent.z = 9000;
        compare(spy.count, 0);
        compare(child.z, 43);

        child.close();
        compare(spy.count, 1);
        compare(child.z, 9001);
    }
}
