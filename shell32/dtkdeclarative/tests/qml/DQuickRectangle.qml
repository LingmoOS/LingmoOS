// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick.Controls 2.11
import org.deepin.dtk 1.0 as D

D.RoundRectangle {
    width: 100;  height: 40
    color: "red"
    radius: 8
    corners: D.RoundRectangle.TopLeftCorner | D.RoundRectangle.TopRightCorner
}
