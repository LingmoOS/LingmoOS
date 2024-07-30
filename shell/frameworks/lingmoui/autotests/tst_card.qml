/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    name: "Cards"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: cardComponent
        LingmoUI.Card {}
    }

    Component {
        id: itemComponent
        Item {}
    }

    Component {
        id: cardWithActionsComponent
        LingmoUI.Card {
            actions: [
                QQC2.Action {
                    text: "QQC2"
                },
                LingmoUI.Action {
                    text: "LingmoUI"
                }
            ]
        }
    }

    function test_init() {
        const card = createTemporaryObject(cardComponent, this);
        verify(card);
    }

    function test_customFooter() {
        const card = createTemporaryObject(cardComponent, this);
        verify(card);

        const item = createTemporaryObject(itemComponent, this);
        verify(item);

        const defaultFooter = card.footer;
        verify(defaultFooter.visible);
        verify(defaultFooter.parent !== null);

        card.footer = item;
        verify(item.visible);
        verify(!defaultFooter.visible);
    }

    function test_cardWithActions() {
        const card = createTemporaryObject(cardWithActionsComponent, this);
        verify(card);
    }
}
