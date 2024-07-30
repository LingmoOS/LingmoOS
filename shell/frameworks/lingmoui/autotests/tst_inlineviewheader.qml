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

    name: "InlineViewHeaderTests"
    when: windowShown

    width: 300
    height: 300
    visible: true

    Component {
        id: emptyComponent
        LingmoUI.InlineViewHeader {}
    }

    Component {
        id: actionableComponent
        LingmoUI.InlineViewHeader {
            readonly property LingmoUI.Action kActionA: LingmoUI.Action {
                text: "LingmoUI Action A"
            }
            readonly property LingmoUI.Action kActionB: LingmoUI.Action {
                text: "LingmoUI Action B"
                visible: false
            }
            readonly property LingmoUI.Action kActionC: LingmoUI.Action {
                text: "LingmoUI Action C"
            }

            actions: [kActionA, kActionB, kActionC]
        }
    }

    function test_sizing() {
        const emptyHeader = createTemporaryObject(emptyComponent, this);
        verify(emptyHeader);
        const labelOnlyHeader = createTemporaryObject(emptyComponent, this, { text: "Name" });
        verify(labelOnlyHeader);
        const multiLinelabelOnlyHeader = createTemporaryObject(emptyComponent, this, {
            text: "First Line followed by\nSecond Line"
        });
        verify(multiLinelabelOnlyHeader);
        const actionsOnlyHeader = createTemporaryObject(actionableComponent, this);
        verify(actionsOnlyHeader);
        const fullHeader = createTemporaryObject(actionableComponent, this, { text: "Name" });
        verify(fullHeader);
        // Let them polish and instantiate delegates
        wait(1100);

        // check that implicit height is more than just vertical padding
        verify(emptyHeader.implicitHeight > emptyHeader.topPadding + emptyHeader.bottomPadding);

        // implicit height stays the same regardless of text label content:
        compare(emptyHeader.implicitHeight, labelOnlyHeader.implicitHeight);

        // label elides instead of wrapping:
        compare(emptyHeader.implicitHeight, multiLinelabelOnlyHeader.implicitHeight);

        // label contributes to the width
        verify(labelOnlyHeader.implicitWidth > labelOnlyHeader.leftPadding + labelOnlyHeader.rightPadding)

        compare(actionsOnlyHeader.implicitHeight, fullHeader.implicitHeight);

        verify(fullHeader.implicitWidth > actionsOnlyHeader.implicitWidth);
        verify(actionsOnlyHeader.implicitWidth > emptyHeader.implicitWidth);
        verify(labelOnlyHeader.implicitWidth > emptyHeader.implicitWidth);
    }
}
