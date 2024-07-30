/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.kconfig.test14

Item {

    property url dir: test.picturesDir
    property double brightness: test.brightness

    Test14 {
        id: test
    }
}
