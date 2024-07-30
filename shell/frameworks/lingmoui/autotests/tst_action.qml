// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest
import LingmoUITestUtils

TestCase {
    name: "Action"

    LingmoUI.Action {
        id: normalAction
    }

    function test_normal(): void {
        compare(normalAction.text, '');
        compare(normalAction.enabled, true);
    }

    LingmoUI.Action {
        id: enabledAction
        fromQAction: ActionData.enabledAction
    }

    function test_enabledAction(): void {
        compare(enabledAction.text, 'Enabled Action');
        compare(enabledAction.enabled, true);
    }

    LingmoUI.Action {
        id: disabledAction
        fromQAction: ActionData.disabledAction
    }

    function test_disabledAction(): void {
        compare(disabledAction.text, 'Disabled Action');
        compare(disabledAction.enabled, false);
    }
}