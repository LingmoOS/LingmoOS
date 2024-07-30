/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects as GE

QQC2.Label {
    layer.effect: GE.DropShadow {
        verticalOffset: 1
        radius: 4.0
        samples: radius * 2 + 1
        spread: 0.35
        color: "black"
    }
}
