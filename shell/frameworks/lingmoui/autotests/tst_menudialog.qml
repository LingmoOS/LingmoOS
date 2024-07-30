/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root

    name: "MenuDialogTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    Component {
        id: menuDialogComponent
        LingmoUI.MenuDialog {
            readonly property LingmoUI.Action actionA: LingmoUI.Action {
                text: "Action A"
            }

            preferredWidth: 200

            actions: [actionA]
        }
    }

    Component {
        id: spyComponent
        SignalSpy {}
    }

    function findChildIf(parent: Item, predicate /*(Item) -> bool*/): Item {
        for (const child of parent.children) {
            if (predicate(child)) {
                return child;
            } else {
                const item = findChildIf(child, predicate);
                if (item !== null) {
                    return item;
                }
            }
        }
        return null;
    }

    function testClosed() {
        const dialog = createTemporaryObject(this, menuDialogComponent);
        verify(dialog);

        const { actionA } = dialog;

        const dialogClosedSpy = createTemporaryObject(this, spyComponent, {
            target: dialog,
            signalName: "closed",
        });
        const actionSpy = createTemporaryObject(this, spyComponent, {
            target: actionA,
            signalName: "triggered",
        });

        dialog.open();
        tryVerify(() => dialog.opened);

        const delegate = findChildIf(dialog.contentItem, item => item.action === actionA) as QQC2.ItemDelegate;
        verify(delegate);

        mouseClick(delegate);
        compare(actionSpy.count, 1);
        compare(dialogClosedSpy.count, 1);
        tryVerify(() => !dialog.visible);
    }
}
