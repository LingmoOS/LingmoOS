// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.11
import org.deepin.dtk 1.0 as D

D.ButtonBox {
    ToolButton {
        text: "Button1"
        checked: true
    }
    ToolButton {
        text: "Button2"
    }
}
