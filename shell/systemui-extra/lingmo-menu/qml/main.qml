/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import AppUI 1.0 as AppUI
import AppControls2 1.0 as AppControls2
import org.lingmo.menu.core 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

Item {
    id: root
    clip: true
    property int animationDuration: menuSetting.get("animationDuration")
    property var normalGeometry: mainWindow.normalRect
    property string currentSearchText: ""
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true;

    Component.onCompleted: {
        mainWindow.fullScreenChanged.connect(enterFullScreen);
        mainWindow.beforeFullScreenExited.connect(exitFullScreen);
        menuSetting.changed.connect(updateAnimationDuration);
    }

    function enterFullScreen() {
        if (mainWindow.isFullScreen) {
            normalHide.start();
            fullShow.start();
            enterFullScreenAnimation.start();
        }
    }

    function exitFullScreen() {
        normalShow.start();
        fullHide.start();
        exitFullScreenAnimation.start();
    }

    function updateAnimationDuration(key) {
        if (key === "animationDuration") {
            root.animationDuration = menuSetting.get("animationDuration");
        }
    }

    LingmoUIItems.StyleBackground {
        id: backgroundMask
        // 初始状态默认为normalScreen
        x: mainWindow.isFullScreen ? 0 : normalGeometry.x
        y: mainWindow.isFullScreen ? 0 : normalGeometry.y
        height: mainWindow.isFullScreen ? root.height : normalGeometry.height
        width: mainWindow.isFullScreen ? root.width : normalGeometry.width
        radius: mainWindow.isFullScreen ? 0 : Platform.Theme.windowRadius

        border.width: mainWindow.isFullScreen ? 0 : 1
        borderColor: Platform.Theme.Text
        borderAlpha: 0.15

        Component.onCompleted: mainWindow.changeWindowBlurRegion(x, y, width, height, radius)
        onRadiusChanged: mainWindow.changeWindowBlurRegion(x, y, width, height, radius)
        onWidthChanged: mainWindow.changeWindowBlurRegion(x, y, width, height, radius)
        onHeightChanged: mainWindow.changeWindowBlurRegion(x, y, width, height, radius)

        ParallelAnimation {
            id: enterFullScreenAnimation
            PropertyAnimation {
                target: backgroundMask; property: "x"
                from: normalGeometry.x; to: 0
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "y"
                from: normalGeometry.y; to: 0
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "height"
                from: normalGeometry.height; to: root.height
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "width"
                from: normalGeometry.width; to: root.width
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }

            onStarted: {
                fullScreenLoader.active = true;
            }
            onFinished: {
                normalScreenLoader.active = false;
            }
        }

        ParallelAnimation {
            id: exitFullScreenAnimation
            PropertyAnimation {
                target: backgroundMask; property: "x"
                from: 0; to: normalGeometry.x
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "y"
                from: 0; to: normalGeometry.y
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "height"
                from: root.height; to: normalGeometry.height
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: backgroundMask; property: "width"
                from: root.width; to: normalGeometry.width
                duration: root.animationDuration; easing.type: Easing.InOutQuad
            }

            onStarted: {
                normalScreenLoader.active = true;
            }
            onFinished: {
                fullScreenLoader.active = false;
                mainWindow.isFullScreen = false;
            }
        }
    }

    Loader {
        id: normalScreenLoader
        focus: !mainWindow.isFullScreen
        active: false
        x: normalGeometry.x
        y: normalGeometry.y
        height: normalGeometry.height
        width: normalGeometry.width
        sourceComponent: normalComponent
        NumberAnimation { id: normalShow; target: normalScreenLoader; properties: "opacity"; from: 0; to: 1; duration: root.animationDuration; easing.type: Easing.InQuint }
        NumberAnimation { id: normalHide; target: normalScreenLoader; properties: "opacity"; from: 1; to: 0; duration: root.animationDuration; easing.type: Easing.OutQuint }
        Component.onCompleted: {
            active = mainWindow.isFullScreen ? false : true;
            opacity = mainWindow.isFullScreen ? 0 : 1;
        }
        Keys.onPressed: {
            item.keyPressed(event);
        }
    }

    Loader {
        id: fullScreenLoader
        x: 0; y: 0
        width: parent.width; height: parent.height
        focus: mainWindow.isFullScreen
        sourceComponent: fullSceenComponent
        ParallelAnimation {
            id: fullShow
            NumberAnimation { target: fullScreenLoader; properties: "opacity"; from: 0; to: 1; duration: root.animationDuration; easing.type: Easing.InQuint }
            NumberAnimation { target: fullScreenLoader; properties: "y"; from: -100; to: 0; duration: root.animationDuration; easing.type: Easing.InOutQuad }
        }
        NumberAnimation { id: fullHide; target: fullScreenLoader; properties: "opacity"; from: 1; to: 0; duration: root.animationDuration; easing.type: Easing.OutQuint }

        Component.onCompleted: {
            active = mainWindow.isFullScreen ? true : false;
            opacity = mainWindow.isFullScreen ? 1 : 0;
        }

        Keys.onPressed: {
            item.keyPressed(event);
        }
    }

    Component {
        id: fullSceenComponent
        AppUI.FullScreenUI {}
    }

    Component {
        id: normalComponent
        AppUI.NormalUI { }
    }
}
