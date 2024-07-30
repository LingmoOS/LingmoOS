/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.pipewire.monitor as Monitor

PlasmoidItem {
    id: root

    switchWidth: LingmoUI.Units.gridUnit * 12
    switchHeight: LingmoUI.Units.gridUnit * 12

    toolTipSubText: if (!monitor.detectionAvailable) {
        return i18nc("@info:tooltip", "Camera indicator is unavailable");
    } else if (monitor.runningCount) {
        return i18ncp("@info:tooltip", "A camera is in use", "%1 cameras are in use", monitor.runningCount);
    } else {
        return i18nc("@info:tooltip", "No camera is in use");
    }

    Plasmoid.icon: if (!monitor.detectionAvailable) {
        return "network-disconnect-symbolic";
    } else if (monitor.idleCount > 0 && monitor.runningCount === 0) {
        return "camera-ready-symbolic";
    } else if (monitor.runningCount > 0) {
        return "camera-on-symbolic";
    } else {
        return "camera-off-symbolic";
    }
    Plasmoid.status: (monitor.idleCount > 0 || monitor.runningCount > 0) ? LingmoCore.Types.ActiveStatus : LingmoCore.Types.HiddenStatus

    Monitor.MediaMonitor {
        id: monitor
        role: Monitor.MediaRole.Camera
    }

    fullRepresentation: FullRepresentation { }
}
