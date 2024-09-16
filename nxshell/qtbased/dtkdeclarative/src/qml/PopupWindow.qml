// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import org.deepin.dtk.impl 1.0 as D

Window {
    property alias blurControl: behindBlur.control

    flags: Qt.Popup
    D.DWindow.enabled: true
    D.DWindow.enableBlurWindow: true

    StyledBehindWindowBlur {
        id: behindBlur
        anchors.fill: parent
    }
}
