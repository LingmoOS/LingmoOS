/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase

    width: 400
    height: 400
    name: "MobilePageRowTest"
    visible: false

    Component {
        id: applicationComponent
        LingmoUI.ApplicationWindow {
            // Mobile pagerow logic branches at 40 gridUnits boundary
            width: LingmoUI.Units.gridUnit * 30
            height: 400
        }
    }

    Component {
        id: pageComponent
        LingmoUI.Page {
            id: page
            property alias closeButton: closeButton
            title: "TestPageComponent"
            QQC2.Button {
                id: closeButton
                anchors.centerIn: parent
                objectName: "CloseDialogButton"
                text: "Close"
                onClicked: page.closeDialog();
            }
        }
    }

    // The following methods are adaptation of QtTest internals

    function waitForWindowActive(window: Window) {
        tryVerify(() => window.active);
    }

    function ensureWindowShown(window: Window) {
        window.requestActivate();
        waitForWindowActive(window);
        wait(0);
    }

    function init() {
        verify(LingmoUI.Settings.isMobile);
    }

    function test_pushDialogLayer() {
        const app = createTemporaryObject(applicationComponent, this);
        verify(app);
        ensureWindowShown(app);

        verify(app.pageStack.layers instanceof QQC2.StackView);
        compare(app.pageStack.layers.depth, 1);
        {
            const page = app.pageStack.pushDialogLayer(pageComponent);
            verify(page instanceof LingmoUI.Page);
            compare(page.title, "TestPageComponent");
            // Wait for it to finish animating
            tryVerify(() => !app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 2);
            mouseClick(page.closeButton);
            tryVerify(() => !app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 1);
        }
        app.width = LingmoUI.Units.gridUnit * 50;
        {
            const page = app.pageStack.pushDialogLayer(pageComponent);
            verify(page instanceof LingmoUI.Page);
            compare(page.title, "TestPageComponent");
            verify(!app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 1);
            mouseClick(page.closeButton);
        }
    }
}
