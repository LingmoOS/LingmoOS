/*
    SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.kirigami as Kirigami

Window {
    id: root
    width: 500
    height: 300
    visible: true
    Kirigami.SelectableLabel {
        anchors.fill: parent
        text: "Hello World"
    }
}
