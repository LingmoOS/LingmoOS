// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

Item {
    id: sliderShow

    property bool autoRun: false

    // 减少重复触发，变更后单独更新图片源
    property alias source: fadeInOutImage.imageSource

    function outSliderShow() {
        showNormal();
        stackView.switchImageView();
    }

    function restart() {
        autoRun = true;
        fadeInOutImage.restart();
    }

    function switchNextImage() {
        if (!IV.GControl.hasNextImage) {
            IV.GControl.firstImage();
        } else {
            IV.GControl.nextImage();
        }
        source = "image://ImageLoad/" + IV.GControl.currentSource + "#frame_" + IV.GControl.currentFrameIndex;
    }

    function switchPreviousImage() {
        if (!IV.GControl.hasPreviousImage) {
            IV.GControl.lastImage();
        } else {
            IV.GControl.previousImage();
        }
        source = "image://ImageLoad/" + IV.GControl.currentSource + "#frame_" + IV.GControl.currentFrameIndex;
    }

    anchors.fill: parent

    Component.onCompleted: {
        source = "image://ImageLoad/" + IV.GControl.currentSource + "#frame_" + IV.GControl.currentFrameIndex;
        showFullScreen();
        restart();
    }

    Timer {
        id: timer

        interval: 3000
        repeat: true
        running: autoRun

        onTriggered: switchNextImage()
    }

    SFadeInOut {
        id: fadeInOutImage

        anchors.fill: parent
    }

    MouseArea {
        id: sliderArea

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent
        cursorShape: "BlankCursor"
        hoverEnabled: true

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                sliderMenu.popup();
            }
        }
        onCursorShapeChanged: sliderCursorTimer.start()
        onDoubleClicked: outSliderShow()
        onMouseXChanged: {
            sliderArea.cursorShape = "ArrowCursor";
        }
        onMouseYChanged: {
            sliderArea.cursorShape = "ArrowCursor";
            if (mouseY > height - 100) {
                showSliderAnimation.start();
            } else {
                hideSliderAnimation.start();
            }
        }

        Timer {
            id: sliderCursorTimer

            interval: 3000 // 设置定时器定时时间为500ms,默认1000ms
            repeat: true // 是否重复定时,默认为false
            running: true // 是否开启定时，默认是false，当为true的时候，进入此界面就开始定时

            onTriggered: sliderArea.cursorShape = "BlankCursor"
        }

        NumberAnimation {
            id: hideSliderAnimation

            duration: 200
            easing.type: Easing.InOutQuad
            from: sliderFloatPanel.y
            property: "y"
            target: sliderFloatPanel
            to: screen.height
        }

        NumberAnimation {
            id: showSliderAnimation

            duration: 200
            easing.type: Easing.InOutQuad
            from: sliderFloatPanel.y
            property: "y"
            target: sliderFloatPanel
            to: screen.height - 80
        }

        FloatingPanel {
            id: sliderFloatPanel

            height: 70
            width: 232

            Component.onCompleted: {
                sliderFloatPanel.x = (screen.width - width) / 2;
                sliderFloatPanel.y = screen.height - 80;
            }

            Row {
                height: 50
                spacing: 10

                anchors {
                    left: parent.left
                    leftMargin: 10
                    top: parent.top
                    topMargin: parent.height / 2 - height / 2
                }

                IconButton {
                    id: sliderPrevious

                    ToolTip.delay: 500
                    ToolTip.text: qsTr("Previous")
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    height: parent.height
                    icon.name: "icon_previous"
                    width: 50

                    onClicked: {
                        switchPreviousImage();
                        autoRun = false;
                    }
                }

                IconButton {
                    id: sliderPause

                    ToolTip.delay: 500
                    ToolTip.text: autoRun ? qsTr("Pause") : qsTr("Play")
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    height: parent.height
                    icon.name: autoRun ? "icon_suspend" : "icon_play_start"
                    width: 50

                    onClicked: {
                        autoRun = !autoRun;
                    }
                }

                IconButton {
                    id: sliderNext

                    ToolTip.delay: 500
                    ToolTip.text: qsTr("Next")
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    height: parent.height
                    icon.name: "icon_next"
                    width: 50

                    onClicked: {
                        switchNextImage();
                        autoRun = false;
                    }
                }

                ActionButton {
                    ToolTip.delay: 500
                    ToolTip.text: qsTr("Exit")
                    ToolTip.timeout: 5000
                    ToolTip.visible: hovered
                    height: parent.height
                    icon.name: "entry_clear"
                    width: 24

                    onClicked: outSliderShow()
                }
            }
        }
    }

    Menu {
        id: sliderMenu

        x: 250
        y: 600

        MenuItem {
            text: autoRun ? qsTr("Pause") : qsTr("Play")

            onTriggered: {
                autoRun = !autoRun;
            }

            // 添加处理快捷键，播放幻灯片时暂停/播放
            Shortcut {
                id: pauseShortCut

                // 进行幻灯片播放时允许响应空格快捷键处理暂停/播放
                enabled: sliderShow.visible
                sequence: "Space"

                onActivated: {
                    autoRun = !autoRun;
                }
            }
        }

        MenuItem {
            text: qsTr("Exit")

            onTriggered: outSliderShow()

            Shortcut {
                sequence: "Esc"

                onActivated: outSliderShow()
            }
        }
    }
}
