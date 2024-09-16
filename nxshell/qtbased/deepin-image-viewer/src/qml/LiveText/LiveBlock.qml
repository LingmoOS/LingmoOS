// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0

Rectangle {
    id: root

    property var charLocations: new Array
    property int endCharIndex: -1
    property int index: 0
    property int startCharIndex: -1

    signal blockMenu
    signal blockMouseMoved(int mouseX, int mouseY)
    signal blockMouseReleased
    signal blockPressed(int mouseX, int mouseY)

    function clearSelect(pressedIndex) {
        if (pressedIndex !== index) {
            liveArea.pressedXAxis = -1;
            rubberBand.visible = false;
        }
    }

    function enableHighLight(enable) {
        highLight.visible = enable;
    }

    function getSelectedText() {
        if (startCharIndex != -1 && endCharIndex != -1) {
            return liveTextAnalyzer.textResult(index, startCharIndex, endCharIndex - startCharIndex + 1);
        } else {
            return "";
        }
    }

    function getSelectedTextCount() {
        if (startCharIndex != -1 && endCharIndex != -1) {
            return endCharIndex - startCharIndex + 1;
        } else {
            return 0;
        }
    }

    function mouseAxisChanged(xAxis, yAxis) {
        liveArea.mouseAxisChanged(xAxis, yAxis);
    }

    function mousePressed(xAxis, yAxis) {
        liveArea.mousePressed(xAxis, yAxis);
    }

    function mouseReleased() {
        liveArea.mouseReleased();
    }

    function selectAll() {
        //1.字符位置
        startCharIndex = 0;
        endCharIndex = charLocations.length - 1;
        //2.橡皮筋框
        rubberBand.visible = true;
        rubberBand.x = 0;
        rubberBand.width = root.width;
    }

    function setCharLocations(newLocation) {
        charLocations = newLocation;
    }

    Image {
        anchors.fill: parent
        fillMode: Image.Tile
        source: "image://liveTextAnalyzer/" + Math.random() + "_" + index
    }

    Rectangle {
        id: highLight

        anchors.fill: parent
        color: "#AABBCC"
        opacity: 0.5
        visible: false
    }

    //鼠标事件
    MouseArea {
        id: liveArea

        property int pressedXAxis: -1

        function flushRubberBandDisplay(mouseXAxis) {
            //计算覆盖范围
            var rangeMin = Math.min(pressedXAxis, mouseXAxis);
            var rangeMax = Math.max(pressedXAxis, mouseXAxis);

            //搜索最小显示位置
            var minDisplayPos = -1;
            var i;
            for (i = 0; i !== charLocations.length - 1; ++i) {
                if (charLocations[i + 1] - rangeMin > (charLocations[i + 1] - charLocations[i]) / 2) {
                    minDisplayPos = charLocations[i];
                    startCharIndex = i;
                    break;
                }
            }
            if (minDisplayPos == -1) {
                startCharIndex = -1;
                endCharIndex = -1;
                rubberBand.visible = false;
                return;
            }

            //搜索最大显示位置
            var maxDisplayPos = -1;
            for (i = charLocations.length - 1; i >= 1; --i) {
                if (rangeMax - charLocations[i - 1] > (charLocations[i] - charLocations[i - 1]) / 2) {
                    maxDisplayPos = charLocations[i];
                    endCharIndex = i - 1;
                    break;
                }
            }
            if (maxDisplayPos == -1) {
                startCharIndex = -1;
                endCharIndex = -1;
                rubberBand.visible = false;
                return;
            }

            //赋值
            rubberBand.x = minDisplayPos;
            rubberBand.width = maxDisplayPos - minDisplayPos;
            rubberBand.visible = true;
        }

        function mouseAxisChanged(xAxis, yAxis) {
            if (pressedXAxis == -1) {
                return;
            }
            if (yAxis < root.y) {
                //光标位于上方，左向右的变为完全不中，右向左的变为完全中
                flushRubberBandDisplay(0);
            } else if (yAxis > root.y + root.height) {
                //光标位于下方，左向右的变为完全中，右向左的变为完全不中
                flushRubberBandDisplay(root.width);
            } else {
                flushRubberBandDisplay(xAxis - root.x);
            }
        }

        function mousePressed(xAxis, yAxis) {
            //鼠标点击区域位于当前block里面
            if (!(xAxis < root.x || xAxis > root.x + root.width || yAxis < root.y || yAxis > root.y + root.height)) {
                pressedXAxis = mouseX;
            } else {
                //优先判断Y方向是否在block的Y轴范围内
                if (yAxis < root.y) {
                    //在触发区域的上方
                    //此时需要初始化为从左向右拉的状态
                    pressedXAxis = 0;
                } else if (yAxis > root.y + root.height) {
                    //在触发区域的下方
                    //此时需要初始化为从右向左拉的状态
                    pressedXAxis = liveArea.x + liveArea.width;
                } else {
                    //与触发区域的Y轴存在交集，可以认为是平齐的，交集值需要后期根据UI的要求调整
                    //此时需要根据X轴的位置做进一步判断
                    if (xAxis < root.x) {
                        //平齐，左侧，左向右
                        pressedXAxis = 0;
                    } else {
                        //平齐，右侧，右向左
                        pressedXAxis = liveArea.x + liveArea.width;
                    }
                }
            }
            rubberBand.visible = false;
        }

        function mouseReleased() {
            pressedXAxis = -1;
        }

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        hoverEnabled: true

        onClicked: mouse => {
            if (mouse.button === Qt.RightButton) {
                blockMenu();
            }
        }
        onDoubleClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                selectAll();
            }
        }
        onMouseXChanged: mouse => {
            if (mouse.button === Qt.RightButton) {
                return;
            }
            blockMouseMoved(mouseX + root.x, mouseY + root.y);
        }
        onMouseYChanged: mouse => {
            if (mouse.button === Qt.RightButton) {
                return;
            }
            blockMouseMoved(mouseX + root.x, mouseY + root.y);
        }
        onPressed: mouse => {
            if (mouse.button === Qt.RightButton) {
                return;
            }
            blockPressed(mouseX + root.x, mouseY + root.y);
        }
        onReleased: mouse => {
            if (mouse.button === Qt.RightButton) {
                return;
            }
            blockMouseReleased();
        }
    }

    LiveBlockRubberBand {
        id: rubberBand

        height: parent.height
        visible: liveArea.pressed
        width: 0
        y: liveArea.y
    }
}
