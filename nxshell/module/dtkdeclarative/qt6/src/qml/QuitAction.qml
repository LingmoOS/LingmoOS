// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11

Action {
    id: quitAction
    text: qsTr("Exit")
    onTriggered: Qt.quit()
}
