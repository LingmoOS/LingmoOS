/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Templates as T
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    name: "SpellCheckAttachedTest"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: emptyComponent
        T.TextArea {}
    }

    Component {
        id: attachedComponent
        T.TextArea {
            LingmoUI.SpellCheck.enabled: true
        }
    }

    function test_defaults() {
        const area = createTemporaryObject(emptyComponent, this);
        verify(area);

        verify(!area.LingmoUI.SpellCheck.enabled);
        area.LingmoUI.SpellCheck.enabled = true;
        verify(area.LingmoUI.SpellCheck.enabled);
    }

    function test_createAttached() {
        const area = createTemporaryObject(attachedComponent, this);
        verify(area);

        verify(area.LingmoUI.SpellCheck.enabled);
    }
}
