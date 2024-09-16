// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick.Window 2.11
import org.deepin.dtk 1.0 as D

Window {
    D.DWindow.enabled: true
    flags: Qt.Window | Qt.WindowMaximizeButtonHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    property var group : group
    D.WindowButtonGroup {
        id: group
    }
}
