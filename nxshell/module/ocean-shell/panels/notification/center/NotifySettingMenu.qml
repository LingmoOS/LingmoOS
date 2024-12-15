// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.lingmo.dtk 1.0
import org.lingmo.ds.notificationcenter

Menu {
    id: root

    closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape

    function toggle()
    {
        if (!visible) {
            console.log("Open menu")
            open()
        } else {
            console.log("Close menu")
            close()
        }
    }
}
