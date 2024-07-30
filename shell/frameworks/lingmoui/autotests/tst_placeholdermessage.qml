/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root

    name: "PlaceholderMessageTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    Component {
        id: placeholderMessageComponent
        LingmoUI.PlaceholderMessage {
            id: message

            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)

            readonly property SignalSpy linkHoveredSpy: SignalSpy {
                target: message
                signalName: "linkHovered"
            }
            readonly property SignalSpy linkActivatedSpy: SignalSpy {
                target: message
                signalName: "linkActivated"
            }
        }
    }

    Component {
        id: helpfulMessageComponent
        LingmoUI.PlaceholderMessage {
            id: message

            property int count: 0

            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)

            helpfulAction: QQC2.Action {
                onTriggered: {
                    message.count += 1
                }
            }
        }
    }

    function hoverAll(item: Item, /*predicate: (x, y) => bool*/ until) {
        for (let x = 0; x < item.width; x += 10) {
            for (let y = 0; y < item.height; y += 10) {
                mouseMove(item, x, y);
                if (until(x, y)) {
                    return Qt.point(x, y);
                }
            }
        }
        return null;
    }

    function test_link() {
        skip("finding links by sweeping with the mouse cursor all over the place seems very unreliable, especially on FreeBSD and Windows TODO: find a better way to find links")
        const href = "some";
        const message = createTemporaryObject(placeholderMessageComponent, this, {
            text: "Attention!",
            explanation: `<a href="${href}">link</a>`,
        });
        verify(message);
        verify(message.linkHoveredSpy.valid);
        verify(message.linkActivatedSpy.valid);

        const point = hoverAll(message, (x, y) => message.hoveredLink === href);
        verify(point !== null);
        compare(message.linkHoveredSpy.count, 1);
        const hoveredLink = message.linkHoveredSpy.signalArguments[0][0];
        compare(hoveredLink, href);

        mouseClick(message, point.x, point.y);
        compare(message.linkActivatedSpy.count, 1);
        const activatedLink = message.linkActivatedSpy.signalArguments[0][0];
        compare(activatedLink, href);
    }

    function test_action() {
        const message = createTemporaryObject(helpfulMessageComponent, this, {
            text: "Attention!",
        });
        const point = hoverAll(message, (x, y) => {
            mouseClick(message, x, y);
            return message.count > 0;
        });
        verify(point !== null);
        compare(message.count, 1);
    }

    function test_disabled_action() {
        const message = createTemporaryObject(helpfulMessageComponent, this, {
            text: "Attention!",
        });
        message.helpfulAction.enabled = false;
        const point = hoverAll(message, (x, y) => {
            mouseClick(message, x, y);
            return message.count > 0;
        });
        verify(point === null);
        compare(message.count, 0);
    }

    function test_null_action() {
        const message = createTemporaryObject(helpfulMessageComponent, this, {
            text: "Attention!",
            helpfulAction: null,
        });
        const point = hoverAll(message, (x, y) => {
            mouseClick(message, x, y);
            return message.count > 0;
        });
        verify(point === null);
        compare(message.count, 0);
    }
}
