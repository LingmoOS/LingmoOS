/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick 2.12

pragma Singleton;

QtObject {
    id: log

    property LoggingCategory generalCategory: LoggingCategory {
        name: "kf.quickcharts.general"
        defaultLogLevel: LoggingCategory.Warning
    }

    property LoggingCategory deprecatedCategory: LoggingCategory {
        name: "kf.quickcharts.deprecated"
        defaultLogLevel: LoggingCategory.Warning
    }

    function deprecated(item, entry, since, message) {
        console.warn(log.deprecatedCategory, "%1::%2 is deprecated (since %3): %4".arg(item).arg(entry).arg(since).arg(message))
    }
}
