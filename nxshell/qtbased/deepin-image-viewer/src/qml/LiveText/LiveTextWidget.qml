// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import "LiveBlockController.js" as BlockLoader

Item {
    id: root

    //block汇总
    property var blockArray: new Array
    property bool currentHasSelect: false

    //清除选择效果，参数为发出此信号的index，-1表示全部清除且只有root可以发出
    signal clearSelect(int pressesIndex)

    //广播信号
    //退出Live Text模式
    signal liveExit

    //鼠标在整个Live区域中移动，此时鼠标必定是保持press状态，参数为当前鼠标的X坐标，当前鼠标的Y坐标
    signal mouseMoveInLive(int moveX, int moveY)

    //鼠标在block中点击，此时要初始化所有block的选择能力，鼠标按下的X坐标，鼠标按下的Y坐标
    signal mousePressedInBlock(int pressX, int pressY)

    //鼠标在block中释放，此时要解除所有block的选择状态
    signal mouseReleasedInBlock

    function clearLive() {
        root.visible = false;
        liveExit();

        // 销毁控件
        for (var i = 0; i != blockArray.length; ++i) {
            var rectDetail = blockArray[i];
            // 信号连接需显式断开
            // 清除选择
            clearSelect.disconnect(rectDetail.clearSelect);
            // 框选相关
            // 广播信号发出
            mousePressedInBlock.disconnect(rectDetail.mousePressed);
            mouseMoveInLive.disconnect(rectDetail.mouseAxisChanged);
            mouseReleasedInBlock.disconnect(rectDetail.mouseReleased);
            //触发广播信号
            rectDetail.blockPressed.disconnect(mousePressedInBlock);
            rectDetail.blockMouseMoved.disconnect(mouseMoveInLive);
            rectDetail.blockMouseReleased.disconnect(mouseReleasedInBlock);
            // 弹出菜单
            rectDetail.blockMenu.disconnect(doPopMenu);
            rectDetail.destroy();
        }
        blockArray = [];
        highlightTextButton.visible = false;

        // 清除选中标记
        currentHasSelect = false;
    }

    function doPopMenu() {
        liveMenu.popup();
    }

    function doTextCopy() {
        var currentData;
        var result = "";
        for (var i = 0; i != blockArray.length; ++i) {
            currentData = blockArray[i].getSelectedText();
            if (currentData.length !== 0) {
                result = result + currentData + "\n";
            }
        }
        IV.FileControl.copyText(result);
    }

    function doTextSelectAll() {
        for (var i = 0; i != blockArray.length; ++i) {
            blockArray[i].selectAll();
        }
        currentHasSelect = haveSelect();
    }

    //目前暂时只能处理对齐的矩形框
    //blocks: QList<QList<float>>
    function drawRect(blocks) {
        if (blockArray.length != 0) {
            liveExit();
            blockArray.length = [];
        }
        for (var id = 0; id !== blocks.length; id++) {
            BlockLoader.createBlockObjects(root);
            var rectDetail = BlockLoader.block;
            rectDetail.index = id;
            rectDetail.width = blocks[id][2] - blocks[id][0];
            rectDetail.height = blocks[id][5] - blocks[id][3];
            rectDetail.x = blocks[id][0];
            rectDetail.y = blocks[id][1];
            rectDetail.setCharLocations(liveTextAnalyzer.charBox(id));
            //控件销毁在 clearLive() 处理(需断开信号连接)
            //清除选择
            clearSelect.connect(rectDetail.clearSelect);
            //框选相关
            //广播信号发出
            mousePressedInBlock.connect(rectDetail.mousePressed);
            mouseMoveInLive.connect(rectDetail.mouseAxisChanged);
            mouseReleasedInBlock.connect(rectDetail.mouseReleased);
            //触发广播信号
            rectDetail.blockPressed.connect(mousePressedInBlock);
            rectDetail.blockMouseMoved.connect(mouseMoveInLive);
            rectDetail.blockMouseReleased.connect(mouseReleasedInBlock);
            //弹出菜单
            rectDetail.blockMenu.connect(doPopMenu);
            //保存
            blockArray.push(rectDetail);
        }
        highlightTextButton.visible = blockArray.length > 0;
        highlightTextButton.isHighlight = false;
    }

    function haveSelect() {
        var have = false;
        for (var i = 0; i != blockArray.length; ++i) {
            if (blockArray[i].getSelectedTextCount() !== 0) {
                have = true;
                break;
            }
        }
        return have;
    }

    visible: false

    onMouseReleasedInBlock: {
        currentHasSelect = haveSelect();
    }

    Menu {
        id: liveMenu

        onVisibleChanged: {
            if (visible == true) {
                copyItem.enabled = haveSelect();
            }
        }

        MenuItem {
            id: copyItem

            text: qsTr("Copy (Ctrl+C)")

            onTriggered: {
                doTextCopy();
            }

            Shortcut {
                enabled: currentHasSelect
                sequence: "Ctrl+C"

                onActivated: {
                    Qt.callLater(doTextCopy);
                }
            }
        }

        MenuItem {
            text: qsTr("Select all (Ctrl+A)")

            onTriggered: {
                doTextSelectAll();
            }

            Shortcut {
                sequence: "Ctrl+A"

                onActivated: {
                    doTextSelectAll();
                }
            }
        }
    }
}
