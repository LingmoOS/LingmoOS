// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

LineEdit {
    id: control

    // 暴露给外部的属性
    readonly property bool isEchoMode: control.echoMode === TextInput.Normal
    property alias echoButtonVisible: echoButton.visible

    function toggleEchoMode() {
        if (control.isEchoMode) {
            control.echoMode = TextInput.Password
        } else {
            control.echoMode = TextInput.Normal
        }
    }

    echoMode: TextInput.Password
    clearButton.anchors.rightMargin: echoButton.visible ? DS.Style.passwordEdit.echoActionSpacing
                                                          + echoButton.width : 0

    ActionButton {
        id: echoButton

        anchors {
            right: control.right
            rightMargin: DS.Style.passwordEdit.echoActionSpacing
            verticalCenter: control.verticalCenter
        }

        width: height
        focusPolicy: Qt.NoFocus

        onClicked: control.toggleEchoMode()
        icon.name: control.isEchoMode ? "entry_password_shown" : "entry_password_hide"
    }
}
