// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D

Action {
    id: helpAction
    text: qsTr("Help")
    onTriggered:  D.ApplicationHelper.handleHelpAction()
}
