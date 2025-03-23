// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0 as D

Window {
    width: 200; height: 200
    D.DWindow.enabled: true

    D.DWindow.windowRadius: 16
    D.DWindow.borderWidth: 1
    D.DWindow.borderColor: "red"
    D.DWindow.shadowRadius: 10
    D.DWindow.shadowOffset: Qt.point(0, 2)
    D.DWindow.shadowColor: "red"
    D.DWindow.translucentBackground: true
    D.DWindow.enableSystemResize: true
    D.DWindow.enableSystemMove: true
    D.DWindow.enableBlurWindow: true
    D.DWindow.alphaBufferSize: 8

}
