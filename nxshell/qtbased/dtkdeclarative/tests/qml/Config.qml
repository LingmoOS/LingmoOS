// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Item {
    id: control
    signal key3Changed()
    D.Config {
        id: exampleConfig
        name: "example"
        subpath: ""
        property string key2
        property string key3 : "1"
        onKey3Changed: control.key3Changed()
    }
    function setKey3(value)
    {
        exampleConfig.key3 = value
    }
}
