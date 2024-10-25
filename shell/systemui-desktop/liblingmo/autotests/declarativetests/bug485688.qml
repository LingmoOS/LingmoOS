// SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtTest

import org.kde.lingmo.components as PC3
import org.kde.lingmo.extras as LingmoExtras

// Make sure the text field and the button have (almost) the same height
TestCase {
    id: root
    width: 640
    height: 480
    when: windowShown

    LingmoExtras.SearchField {
        id: searchField
    }
    PC3.ToolButton {
        id: pinButton
        icon.name: "window-pin"
        text: "Keep Open"
        display: PC3.ToolButton.IconOnly
    }

    function test_compareHeight() {
        verify(searchField.implicitBackgroundHeight > 0);
        compare(searchField.implicitBackgroundHeight, pinButton.implicitBackgroundHeight + 4);
    }
}
