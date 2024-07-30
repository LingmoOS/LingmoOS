/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.pipewire.monitor as Monitor

Item {
    id: root
    readonly property alias monitor: monitor
    readonly property alias count: repeater.count
    property int state: -1
    Instantiator {
        id: repeater
        model: Monitor.MediaMonitor {
            id: monitor
            role: Monitor.MediaMonitor.Music
        }
        Binding {
            required property int state
            target: root
            property: "state"
            value: state
        }
    }
}
