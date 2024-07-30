/*
 *   SPDX-FileCopyrightText: 2019 Luca Carlon <carlon.luca@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

MenuItem {
    property var _sequence: []
    property alias _text: actionElement.text
    property alias _enabled: actionElement.enabled
    property alias _iconName: actionElement.icon.name

    Shortcut {
        id: shortcutElement
        sequences: [_sequence]
        enabled: false
    }

    action: Action {
        id: actionElement
        shortcut: shortcutElement.sequences ? shortcutElement.sequences[0] : null
    }
}
