/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui.delegates as KD
import QtTest

TestCase {
    name: "DelegatesTest"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: subtitleDelegate
        KD.SubtitleDelegate {}
    }

    Component {
        id: checkSubtitleDelegate
        KD.CheckSubtitleDelegate {}
    }

    Component {
        id: radioSubtitleDelegate
        KD.RadioSubtitleDelegate {}
    }

    Component {
        id: switchSubtitleDelegate
        KD.SwitchSubtitleDelegate {}
    }

    function test_create() {
        failOnWarning(/error/i);
        {
            const delegate = createTemporaryObject(subtitleDelegate, this);
            verify(delegate);
        }
        {
            const delegate = createTemporaryObject(checkSubtitleDelegate, this);
            verify(delegate);
        }
        {
            const delegate = createTemporaryObject(radioSubtitleDelegate, this);
            verify(delegate);
        }
        {
            const delegate = createTemporaryObject(switchSubtitleDelegate, this);
            verify(delegate);
        }
    }
}
