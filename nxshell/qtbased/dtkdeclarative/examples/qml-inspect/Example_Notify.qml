// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Column {
    id: control

    property string longMessage: "Copyright 2014-2019 Adobe (http://www.adobe.com/), with Reserved FontName 'Source'. Source is a trademark of Adobe in the United States and/or other countries."
    property string shortMessage: "short message"

    Button {
        text: "FloatingMessage"
        property int count: 0
        onClicked: {
            if (count % 2) {
                DTK.sendMessage(control, "message" + count)
                DTK.sendMessage(Window.window, "message" + count, "music", 4000, "type1")
                DTK.sendMessage(Window.window, "message" + count, "video", -1)
            } else {
                DTK.sendMessage(control, floatingMsgCom, {content: shortMessage}, -1)
                DTK.sendMessage(Window.window, floatingMsgCom, {content: longMessage, iconName: "music"}, -1)
            }
            count++
        }
    }

    Component {
        id: floatingMsgCom
        FloatingMessage {
            id: floatMsg
            contentItem: RowLayout {
                spacing: 0

                Label {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: floatMsg.message.content
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    Layout.alignment: Qt.AlignVCenter
                    font: DTK.fontManager.t6
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }

                Button {
                    text: "reload"
                    font: DTK.fontManager.t5
                    Layout.alignment: Qt.AlignVCenter
                }
            }
//            button: Button {
//                text: "close"
//                onClicked: {
//                    console.log("close clicked")
//                    DTK.closeMessage(floatMsg)
//                }
//            }
        }
    }
}
