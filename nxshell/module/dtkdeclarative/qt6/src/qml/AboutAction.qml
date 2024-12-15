// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11

Action {
    id: control
    text: qsTr("About")
    property Component aboutDialog
    property QtObject __object
    onTriggered: {
        if (aboutDialog) {
            if (!__object) {
                __object = aboutDialog.createObject(parent)
            }
            __object.show()
        }
    }
    Component.onDestruction: __object && __object.destroy()
}
