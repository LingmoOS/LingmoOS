/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

import "." as Impl

Rectangle {
    color: Qt.rgba(0,0,0,0.4)
    Behavior on opacity {
        OpacityAnimator {
            duration: LingmoUI.Units.longDuration
        }
    }
}
