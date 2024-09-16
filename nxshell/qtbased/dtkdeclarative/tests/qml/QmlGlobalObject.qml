// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick.Controls 2.12
import org.deepin.dtk 1.0 as D

Control {
    palette: D.DTK.palette
    property var color1: D.DTK.makeColor(D.Color.Highlight)
    property var color2: D.DTK.makeColor("red")
}
