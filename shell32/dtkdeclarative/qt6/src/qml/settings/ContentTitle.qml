// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.settings 1.0 as Settings
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk 1.0

Label {
    text: Settings.SettingsGroup.name
    font: __getFont(Settings.SettingsGroup.level)
    textFormat: Text.PlainText
    verticalAlignment: Qt.AlignVCenter
    topPadding: DS.Style.settings.navigation.textVPadding
    bottomPadding: DS.Style.settings.navigation.textVPadding
    anchors {
        left: parent.left
        leftMargin: __getMargin(Settings.SettingsGroup.level)
        right: parent.right
        rightMargin: __getMargin(Settings.SettingsGroup.level)
    }

    function __getFont(level) {
        switch(level) {
        case 0:
            return D.DTK.fontManager.t4
        case 1:
            return D.DTK.fontManager.t5
        }
        return D.DTK.fontManager.t6
    }
    function __getMargin(level) {
        switch(level) {
        case 0:
            return DS.Style.settings.title.marginL1
        case 1:
            return DS.Style.settings.title.marginL2
        }
        return DS.Style.settings.title.marginLOther
    }
}
