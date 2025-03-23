// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

TextField {
    id: control
    readonly property alias clearButton: clearBtn

    rightPadding: clearBtn.active ? (clearBtn.width + clearBtn.anchors.rightMargin) : 0
    selectByMouse: true

    Loader {
        id: clearBtn
        active: control.text.length !== 0
        width: height
        height: control.background.implicitHeight
        anchors {
            right: control.right
            verticalCenter: control.verticalCenter
        }

        sourceComponent: ActionButton {
            focusPolicy: Qt.NoFocus
            icon {
                name: "entry_clear"
                width: DS.Style.edit.actionIconSize
                height: DS.Style.edit.actionIconSize
            }
            onClicked: control.clear()
        }
    }
}
