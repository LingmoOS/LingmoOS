/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import org.kde.lingmoui 2 as LingmoUI

// WINERROR 1459: This Operation Requires Interactive Window Station
Window {
    width: 100
    height: 100
    visible: true
    readonly property real longDuration: LingmoUI.Units.longDuration
}
