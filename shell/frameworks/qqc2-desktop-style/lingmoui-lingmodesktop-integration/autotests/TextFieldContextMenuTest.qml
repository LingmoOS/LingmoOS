/*
    SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.lingmoui as LingmoUI

Window {
    id: root
    width: 500
    height: 300
    visible: true
    LingmoUI.SelectableLabel {
        anchors.fill: parent
        text: "Hello World"
    }
}
