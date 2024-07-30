/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

Loader {
    width: parent.width
    height: parent.height

    function open() {
        if (item) {
            item.open()
        } else {
            active = true;
        }
    }

    onLoaded: item.open()

    active: false
    asynchronous: true
}
