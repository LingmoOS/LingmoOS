// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk 1.0

DialogWindow {
    id: dialog

    property alias fileName: textMetics.text
    property int nameMaxWidth: 200

    // 操作结束 true: 允许删除 false: 取消
    signal finished(bool ret)

    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowStaysOnTopHint
    height: 180
    maximumHeight: 180
    maximumWidth: 400
    minimumHeight: 180
    minimumWidth: 400
    modality: Qt.ApplicationModal
    visible: false
    width: 400

    header: DialogTitleBar {
        enableInWindowBlendBlur: true

        // 仅保留默认状态，否则 hover 上会有变化效果
        icon {
            mode: DTK.NormalState
            name: "user-trash-full-opened"
        }
    }

    Component.onCompleted: {
        setX(window.x + window.width / 2 - width / 2);
        setY(window.y + window.height / 2 - height / 2);
        show();
    }
    onClosing: {
        finished(false);
    }

    ColumnLayout {
        height: parent.height
        width: parent.width

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.maximumHeight: 80
            Layout.minimumHeight: 80
            Layout.preferredHeight: 80

            Label {
                id: notifyText

                property Palette textColor: Palette {
                    normal: ("black")
                    normalDark: ("white")
                }

                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                color: ColorSelector.textColor
                font: DTK.fontManager.t5
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Cannot move \"%1\" to the trash. Do you want to permanently delete it?").arg(textMetics.elidedText)
                wrapMode: Text.Wrap

                TextMetrics {
                    id: textMetics

                    elide: Text.ElideMiddle
                    elideWidth: nameMaxWidth
                    font: notifyText.font
                }
            }

            Label {
                id: messageText

                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                font: DTK.fontManager.t6
                text: qsTr("This action cannot be undone")
                width: dialog.width
            }
        }

        Row {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillHeight: true
            spacing: 10

            Button {
                height: 36
                text: qsTr("Cancel")
                width: 185

                onClicked: {
                    finished(false);
                }
            }

            RecommandButton {
                height: 36
                text: qsTr("Confirm")
                width: 185

                onClicked: {
                    finished(true);
                }
            }
        }
    }
}
