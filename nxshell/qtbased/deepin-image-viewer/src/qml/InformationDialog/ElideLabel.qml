// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.deepin.dtk 1.0

// 提供自动收缩的标签，鼠标Hover后显示全文，这是由于目前 ToolTip 效果不佳调整的
// 后续 ToolTip 修改后调整回标准方式调用
Label {
    id: showlabel

    // 不超过对话框的边距
    property int maxTipsWidth: Window.width - 20
    property alias sourceText: textMetics.text
    property color tipsColor

    font: DTK.fontManager.t8
    text: textMetics.elidedText
    textFormat: Text.PlainText
    visible: sourceText

    TextMetrics {
        id: textMetics

        elide: Text.ElideMiddle
        elideWidth: showlabel.width
        font: showlabel.font
    }

    Loader {
        active: textMetics.width > showlabel.width
        anchors.fill: parent

        sourceComponent: MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onExited: {
                tip.visible = false;
            }
            onMouseXChanged: {
                if (textMetics.width < showlabel.width) {
                    tip.visible = false;
                } else {
                    tip.visible = true;
                }
            }

            ToolTip {
                id: tip

                // 此处代码并非设置背景，而是由palette的变更信号触发 ColorSelector.controlTheme 的更新
                palette.window: DTK.themeType === ApplicationHelper.LightType ? "white" : "black"
                parent: parent
                text: showlabel.sourceText
                visible: parent.focus
                // 调整部分边距，防止内容折叠
                width: Math.min(showlabel.maxTipsWidth, lineTextMetics.width + 12)
                y: showlabel.y + 20

                contentItem: Text {
                    color: tipsColor
                    font: DTK.fontManager.t8
                    horizontalAlignment: Text.AlignLeft
                    text: showlabel.sourceText
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }

                TextMetrics {
                    id: lineTextMetics

                    elide: Text.ElideNone
                    font: DTK.fontManager.t8
                    text: showlabel.sourceText
                }
            }
        }
    }
}
