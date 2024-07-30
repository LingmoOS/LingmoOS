/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.kde.lingmoui as LingmoUI
import QtTest
import "../tests"

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "KeyboardNavigation"

    Component {
        id: mainComponent
        KeyboardTest {
            id: window

            width: 480
            height: 360

            readonly property SignalSpy spyLastKey: SignalSpy {
                target: window.pageStack.currentItem
                signalName: "lastKeyChanged"
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

    function test_press() {
        const window = createTemporaryObject(mainComponent, this);
        verify(window);
        const spy = window.spyLastKey;
        verify(spy.valid);

        ensureWindowShown(window);

        compare(window.pageStack.depth, 2);
        compare(window.pageStack.currentIndex, 1);

        let keyCount = 0;

        keyClick("A");
        keyCount += 1;
        compare(spy.count, keyCount);
        compare(window.pageStack.currentItem.lastKey, "A");

        keyClick(Qt.Key_Left, Qt.AltModifier);
        keyCount += 1;
        compare(spy.count, keyCount);
        compare(window.pageStack.currentIndex, 0);
        compare(window.pageStack.currentItem.lastKey, "");

        keyClick("B")
        keyCount += 1;
        compare(spy.count, keyCount);
        compare(window.pageStack.currentItem.lastKey, "B");
    }
}
