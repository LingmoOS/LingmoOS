// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import test 1.0

Rectangle {
    width: 200; height: 200
    color: "red"
    DSoftwareRoundedImageNodeItem {
        anchors.fill: parent
    }
}
