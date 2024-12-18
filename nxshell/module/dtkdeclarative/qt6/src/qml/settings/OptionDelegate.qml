// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.lingmo.dtk.settings 1.0
import org.lingmo.dtk 1.0

RowLayout {
    id: control
    width: parent.width - parent.leftPadding - parent.rightPadding

    property bool leftVisible: true
    Label {
        id: label
        visible: leftVisible
        text: SettingsOption.name
        verticalAlignment: Text.AlignVCenter
    }
    Item {
        visible: leftVisible
        width: 20
    }
}
