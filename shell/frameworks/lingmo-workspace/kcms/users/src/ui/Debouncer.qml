/*
    SPDX-FileCopyrightText: 2022 Janet Blackquill <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import org.kde.lingmoui 2.16 as LingmoUI

Timer {
    property bool isTriggered: false

    function reset() {
        isTriggered = false;
        restart();
    }

    interval: LingmoUI.Units.humanMoment

    onTriggered: {
        isTriggered = true;
    }
}
