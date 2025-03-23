// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.settings 1.0
import org.deepin.dtk.style 1.0 as DS

Rectangle {
    anchors {
        left: parent.left
        leftMargin: __getMargin(SettingsGroup.level)
        right: parent.right
        rightMargin: __getMargin(SettingsGroup.level)
    }
    height: childrenRect.height
    radius: DS.Style.control.radius

    function __getMargin(level) {
        switch(level) {
        case 0:
            return DS.Style.settings.content.marginL1
        case 1:
            return DS.Style.settings.content.marginL2
        }
        return DS.Style.settings.content.marginOther
    }
}
