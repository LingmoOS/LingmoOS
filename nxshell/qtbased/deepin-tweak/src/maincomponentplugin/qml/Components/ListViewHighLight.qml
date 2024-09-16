// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.4
import "../Utils"

Rectangle {
    color: palette.highlight
    radius: Utils.listRadius
    Rectangle {
        color: palette.highlightedText
        width: 5
        height: 20
        radius: 5
        x: 5
        y: 8
    }
}
