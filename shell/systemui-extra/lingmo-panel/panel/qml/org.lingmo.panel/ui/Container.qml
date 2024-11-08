/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.quick.container 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0
import org.lingmo.panel.impl 1.0 as Panel

WidgetContainerItem {
    id: containerItem
    property int orientation: WidgetContainer.orientation
    property bool isHorizontal: orientation === Types.Horizontal
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true;
    property int trayAreaMaxLength: containerItem.WidgetContainer.screen === null
                              ? 0
                              : isHorizontal
                                ? containerItem.WidgetContainer.screen.size.width * 0.382
                                : containerItem.WidgetContainer.screen.size.height * 0.382
    property int calendarLength: 0
    property int showDesktopLength: 0
    property int systemTrayLength:  Math.max(0, trayAreaMaxLength - calendarLength - showDesktopLength)

    onParentChanged: {
        anchors.fill = parent
    }

    ContentWindow {
        id: panelPosIndicator
        screen: containerItem.WidgetContainer.screen
        position: Types.Bottom
        flags: Qt.Window | Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint
        windowType: WindowType.SystemWindow

        property bool hideIndicator: false
        property bool isHorizontal: (position === Types.BottomCenter || position === Types.TopCenter)
        property int  newPanelPos: Types.Center

        enableBlurEffect: !hideIndicator

        StyleBackground {
            radius: containerItem.WidgetContainer.radius
            paletteRole: Theme.BrightText
            opacity: panelPosIndicator.hideIndicator ? 0 : 1
            width: {
                if (panelPosIndicator.isHorizontal) {
                    return containerItem.WidgetContainer.screen.availableGeometry.width - 6;
                }
                return containerItem.isHorizontal ? containerItem.height : containerItem.width;
            }

            height: {
                if (panelPosIndicator.isHorizontal) {
                    return containerItem.isHorizontal ? containerItem.height : containerItem.width;
                }
                return containerItem.WidgetContainer.screen.availableGeometry.height - 6;
            }
        }
    }

    StyleBackground {
        anchors.fill: parent
        anchors {
            leftMargin: containerItem.WidgetContainer.margin.left
            topMargin: containerItem.WidgetContainer.margin.top
            rightMargin: containerItem.WidgetContainer.margin.right
            bottomMargin: containerItem.WidgetContainer.margin.bottom
        }
        border.width: 0
        radius: containerItem.WidgetContainer.radius

        MouseArea {
            anchors.fill: parent
            hoverEnabled: panel.enableCustomSize
            enabled: !panel.lockPanel

            property point startPoint: "-1, -1"
            property bool isResize: false
            property real screenAngle: 0
            property point prePoint: Qt.point(-1, -1)

            // 计算夹角度数，对边b, 邻边a
            function getAngle(b, a) {
                return Math.atan(b / a) * 180 / Math.PI;
            }

            onExited: {
                if (!pressed) {
                    panel.changeCursor("");
                }
            }

            onReleased: {
                panel.changeCursor("");
                let visible = panelPosIndicator.visible;
                panelPosIndicator.hide();
                if (visible && !panel.lockPanel && (panelPosIndicator.newPanelPos !== containerItem.WidgetContainer.position)) {
                    panel.changePanelPos(panelPosIndicator.newPanelPos);
                }
            }

            // 计算按下点在本地坐标中出发何种操作
            function checkPointOpt(point) {
                switch (containerItem.WidgetContainer.position) {
                    case Types.Left:
                        return (point.x > (width - 2)) ? "resizeHor" : "";
                    case Types.Top:
                        return (point.y > (height - 2)) ? "resizeVec" : "";
                    case Types.Right:
                        return (point.x < 2) ? "resizeHor" : "";
                    case Types.Bottom:
                        return (point.y < 2) ? "resizeVec" : "";
                    default:
                        break
                }

                return "";
            }

            onPressed: (event) => {
                let localPos = Qt.point(event.x, event.y);
                // 按下的起始点
                startPoint = mapToGlobal(localPos);
                prePoint = mapToGlobal(localPos);

                let opt = checkPointOpt(localPos);
                isResize = (opt !== "") && panel.enableCustomSize;

                let screenSize = containerItem.WidgetContainer.screen.size;
                // 屏幕对角线夹角与右边的夹角，右上角和右下角相同
                screenAngle = getAngle(screenSize.width/2, screenSize.height/2);

                if (!isResize) {
                    if (panel.lockPanel) {
                        event.accepted = false;
                    } else {
                        // 切换鼠标样式
                        panel.changeCursor("move");
                    }
                }
            }

            onPositionChanged: (event) => {
                let localPos = Qt.point(event.x, event.y);
                if (!pressed) {
                    // 悬浮状态，切换鼠标样式
                    if (containsMouse) {
                        panel.changeCursor(checkPointOpt(localPos));
                    }
                    return;
                }

                let globalPos = mapToGlobal(localPos);

                if (isResize) {
                    // 拖拽自定义大小功能
                    let distance = 0;
                    switch (containerItem.WidgetContainer.position) {
                        case Types.Left:
                            distance = globalPos.x - prePoint.x;
                            break;
                        case Types.Right:
                            distance = prePoint.x - globalPos.x;
                            break;
                        case Types.Top:
                            distance = globalPos.y - prePoint.y;
                            break;
                        case Types.Bottom:
                            distance = prePoint.y - globalPos.y;
                            break;
                        default:
                            return;
                    }

                    if (distance !== 0) {
                        if (distance > 0) {
                            panel.customPanelSize(Math.ceil(distance));
                        } else if (distance < 0) {
                            panel.customPanelSize(-Math.ceil(Math.abs(distance)));
                        }
                        startPoint = globalPos;
                    }

                } else {
                    // 拖拽换位功能
                    if (Math.abs(startPoint.x - globalPos.x) < 20 || Math.abs(startPoint.y - globalPos.y) < 20) {
                        panelPosIndicator.newPanelPos = containerItem.WidgetContainer.position;
                        panelPosIndicator.hideIndicator = true;
                        return;
                    }

                    // move
                    let screenRect = containerItem.WidgetContainer.screen.geometry;
                    let b = globalPos.x - screenRect.x;
                    let a1 = globalPos.y - screenRect.y;
                    let a2 = screenRect.y + screenRect.height - globalPos.y;

                    if (b < 0 || b > screenRect.width) {
                        return;
                    }

                    if (a1 < 0 || a1 > screenRect.height)  {
                        return;
                    }

                    if (a2 < 0 || a2 > screenRect.height)  {
                        return;
                    }

                    let angleTop = getAngle(b, a1);
                    let angleBottom = getAngle(b, a2);

                    let pos;
                    if (angleTop <= screenAngle && angleBottom < screenAngle) {
                        pos = Types.LeftCenter;
                        panelPosIndicator.newPanelPos = Types.Left;
                    } else if (angleTop > screenAngle && angleBottom <= screenAngle) {
                        pos = Types.TopCenter;
                        panelPosIndicator.newPanelPos = Types.Top;
                    } else if (angleTop >= screenAngle && angleBottom > screenAngle) {
                        pos = Types.RightCenter;
                        panelPosIndicator.newPanelPos = Types.Right;
                    } else if (angleTop < screenAngle && angleBottom >= screenAngle) {
                        pos = Types.BottomCenter;
                        panelPosIndicator.newPanelPos = Types.Bottom;
                    } else {
                        return;
                    }

                    if (panelPosIndicator.newPanelPos === containerItem.WidgetContainer.position) {
                        // TODO: wlcom环境调用hide会导致mouseArea的pressed变为false
                        // panelPosIndicator.hide();
                        panelPosIndicator.hideIndicator = true;
                    } else {
                        panelPosIndicator.hideIndicator = false;
                        panelPosIndicator.show();
                        panelPosIndicator.position = pos;
                    }
                }
                prePoint = mapToGlobal(localPos);
            }
        }

        GridLayout {
            id: mainLayout
            z: 10
            anchors.fill: parent
            anchors.margins: 2
            anchors {
                leftMargin: containerItem.WidgetContainer.padding.left
                topMargin: containerItem.WidgetContainer.padding.top
                rightMargin: containerItem.WidgetContainer.padding.right
                bottomMargin: containerItem.WidgetContainer.padding.bottom
            }

            rows: containerItem.isHorizontal ? 1 : repeater.count
            columns: containerItem.isHorizontal ? repeater.count : 1
            rowSpacing: 4
            columnSpacing: 4

            Repeater {
                id: repeater
                model: Panel.WidgetSortModel {
                    // TODO: use config
                    widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                    sourceModel: containerItem.widgetItemModel
                }
                delegate: widgetLoaderComponent
            }
        }

        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                property int index: model.index
                property Item widgetItem: model.widgetItem
                property string itemId: widgetItem.Widget.id

                Layout.fillWidth: widgetItem && widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem && widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem.Layout.minimumWidth
                Layout.minimumHeight: widgetItem.Layout.minimumHeight

                Layout.preferredWidth: widgetItem.Layout.preferredWidth
                Layout.preferredHeight: widgetItem.Layout.preferredHeight

                Layout.maximumWidth: {
                    if (itemId === "org.lingmo.systemTray" && isHorizontal)  {
                        return containerItem.systemTrayLength;
                    }

                    return widgetItem.Layout.maximumWidth ? widgetItem.Layout.maximumWidth : mainLayout.width;
                }
                Layout.maximumHeight: {
                    if (itemId === "org.lingmo.systemTray" && !isHorizontal) {
                        return containerItem.systemTrayLength;
                    }
                    return widgetItem.Layout.maximumHeight ? widgetItem.Layout.maximumHeight : mainLayout.height;
                }

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = widgetParent

                        if (itemId === "org.lingmo.panel.calendar") {
                            containerItem.calendarLength = Qt.binding(function(){
                                return containerItem.isHorizontal ? width : height;
                            })
                        }
                        if (itemId === "org.lingmo.panel.showDesktop") {
                            containerItem.showDesktopLength = Qt.binding(function(){
                                return containerItem.isHorizontal ? width : height;
                            })
                        }
                    }
                }
            }
        }
    }
}
