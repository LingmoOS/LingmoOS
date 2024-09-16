// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.deepin.dtk 1.0 as D

RowLayout {
    property var bubble
    signal actionInvoked(var actionId)

    D.Button {
        Layout.preferredWidth: 60
        text: bubble.firstActionText
        onClicked: {
            actionInvoked(bubble.firstActionId)
        }
    }

    Loader {
        active: bubble.actionTexts.length > 0
        sourceComponent: bubble.actionTexts.length <= 1 ? singleAction : multiActions
    }
    Component {
        id: singleAction
        D.Button {
            width: 60
            text: bubble.actionTexts[0]
            onClicked: {
                actionInvoked(bubble.actionIds[index])
            }
        }
    }
    Component {
        id: multiActions
        ComboBox {
            width: 80
            model: bubble.actionTexts
            delegate: Button {
                width: 60
                text: modelData
                onClicked: {
                    actionInvoked(bubble.actionIds[index])
                }
            }
        }
    }
}
