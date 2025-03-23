// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Column {
    id: panel
    spacing: 20
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "类型1：可以操作的进度条，点击可以进行暂停，进度条内有文字。目前只有控制中心更新部分用了。类型2：
               所有需要用到进度条的地方，这种进度条不可以操作，而是一种状态的指示，告诉用户当前完成了多少或者
               使用了多少的一个比例。"
        horizontalAlignment: Qt.AlignHCenter
    }

    RowLayout {
        spacing: 40
        ColumnLayout {
            spacing: 30
            ProgressBar {
                from: 0
                to: 100
                value: 45
                formatText: ("已下载 45%（点击暂停）")
            }

            ProgressBar {
                Layout.preferredHeight: 8
                from: 0
                to: 100
                value: 52
            }

            ProgressBar {
                Layout.preferredHeight: 18
                from: 0
                to: 100
                value: 52
            }
        }

        ColumnLayout {
            spacing: 30
            ProgressBar {
                indeterminate: true
                formatText: ("正在准备（点击终止）")
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        parent.animationStop = !parent.animationStop
                    }
                }
            }

            ProgressBar {
                from: 0
                to: 100
                Layout.preferredHeight: 8
                indeterminate: true
            }

            ProgressBar {
                Layout.preferredHeight: 18
                indeterminate: true
            }
        }


        RowLayout {
            Item {
                Layout.preferredWidth: 96
                Layout.preferredHeight: 96

                DciIcon {
                    id: icon1
                    anchors.fill: parent
                    name: "music"
                }

                EmbeddedProgressBar {
                    anchors.horizontalCenter: icon1.horizontalCenter
                    anchors.top: icon1.bottom
                    width: 48
                    height: 6

                    from: 0
                    to: 100
                    value: 58
                }
            }

            DciIcon {
                id: icon2
                Layout.preferredWidth: 96
                Layout.preferredHeight: 96
                name: "music"

                EmbeddedProgressBar {
                    anchors.horizontalCenter: icon2.horizontalCenter
                    anchors.bottom: icon2.bottom
                    anchors.bottomMargin: 12
                    width: 48
                    height: 6

                    from: 0
                    to: 100
                    value: 58
                }
            }

            WaterProgressBar {
                value: 56
            }
        }
    }
}
