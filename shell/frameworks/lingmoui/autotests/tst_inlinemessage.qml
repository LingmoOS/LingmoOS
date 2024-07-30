/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase

    name: "InlineMessageTests"
    when: windowShown

    width: 300
    height: 300
    visible: true

    Component {
        id: inlineMessageComponent
        LingmoUI.InlineMessage {
            id: message

            readonly property SignalSpy linkHoveredSpy: SignalSpy {
                target: message
                signalName: "linkHovered"
            }
            readonly property SignalSpy linkActivatedSpy: SignalSpy {
                target: message
                signalName: "linkActivated"
            }

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            visible: true
        }
    }

    function hoverAll(item: Item, /*predicate*/ until) {
        for (let x = 0; x < item.width; x += 10) {
            for (let y = 0; y < item.height; y += 10) {
                mouseMove(item, x, y);
                if (until()) {
                    return Qt.point(x, y);
                }
            }
        }
        return null;
    }

    function test_link() {
        skip("finding links by sweeping with the mouse cursor all over the place seems very unreliable, especially on FreeBSD and Windows TODO: find a better way to find links")
        const href = "some";
        const message = createTemporaryObject(inlineMessageComponent, this, {
            text: `<a href="${href}">link</a>`,
        });
        verify(message);
        verify(message.linkHoveredSpy.valid);
        verify(message.linkActivatedSpy.valid);

        const point = hoverAll(message, () => message.hoveredLink === href);
        verify(point !== null);
        compare(message.linkHoveredSpy.count, 1);
        const hoveredLink = message.linkHoveredSpy.signalArguments[0][0];
        compare(hoveredLink, href);

        mouseClick(message, point.x, point.y);
        compare(message.linkActivatedSpy.count, 1);
        const activatedLink = message.linkActivatedSpy.signalArguments[0][0];
        compare(activatedLink, href);
    }

    function test_hoveredLink_is_readonly() {
        const message = createTemporaryObject(inlineMessageComponent, this);
        verify(message);
        compare(message.hoveredLink, "");
        let failed = false;
        try {
            message.hoveredLink = "something";
        } catch (e) {
            failed = true;
        }
        verify(failed);
    }
}
