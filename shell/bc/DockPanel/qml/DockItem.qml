import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import Lingmo.Dock 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property bool isLeft: Settings.direction === DockSettings.Left
    property bool isRight: Settings.direction === DockSettings.Right
    property bool isBottom: Settings.direction === DockSettings.Bottom

    property var iconSize: root.isHorizontal ? control.height * iconSizeRatio
                                             : control.width * iconSizeRatio

    property bool draggable: false
    property int dragItemIndex

    property alias icon: icon
    property alias mouseArea: iconArea
    property alias dropArea: iconDropArea

    property bool enableActivateDot: true
    property bool isActive: false

    property var popupText

    property double iconSizeRatio: 0.8
    property var iconName

    property bool dragStarted: false

    signal positionChanged()
    signal released()
    signal pressed(var mouse)
    signal pressAndHold(var mouse)
    signal clicked(var mouse)
    signal rightClicked(var mouse)
    signal doubleClicked(var mouse)
    signal dropped(var drop)

    Drag.active: mouseArea.drag.active && control.draggable
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.MoveAction
    Drag.hotSpot.x: icon.width / 2
    Drag.hotSpot.y: icon.height / 2

    Drag.onDragStarted:  {
        dragStarted = true
    }

    Drag.onDragFinished: {
        dragStarted = false
    }

    LingmoUI.IconItem {
        id: icon
        anchors.centerIn: parent
        width: control.iconSize
        height: control.iconSize
        source: iconName

        visible: !dragStarted

        enabled: true
        smooth: true

        transform: Scale {
            id: iconScale
            origin.x: icon.width / 2
            origin.y: icon.height / 2
            xScale: 1.0
            yScale: 1.0

            // 定义缩放变换的动画行为
            Behavior on xScale {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutQuad
                }
            }
            Behavior on yScale {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutQuad
                }
            }
        }

        ColorOverlay {
            id: iconColorize
            anchors.fill: icon
            source: icon
            color: "#000000"
            opacity: iconArea.pressed && !mouseArea.drag.active ? 0.4 : 0
        }
    }

    DropArea {
        id: iconDropArea
        anchors.fill: icon
        enabled: draggable
        onDropped: control.dropped(drop)
    }

    MouseArea {
        id: iconArea
        anchors.fill: icon
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        drag.axis: Drag.XAndYAxis

        // 处理鼠标悬停事件
        onEntered: {
            // 放大图标
            iconScale.xScale = 1.23
            iconScale.yScale = 1.23
        }

        onExited: {
            // 恢复图标大小
            iconScale.xScale = 1.0
            iconScale.yScale = 1.0
        }

        onClicked: {
            iconScale.xScale = 1.1
            iconScale.yScale = 1.1
            iconScale.xScale = 1.23
            iconScale.yScale = 1.23
            if (mouse.button === Qt.RightButton)
                control.rightClicked(mouse)
            else
                control.clicked(mouse)
        }

        onPressed: {
            iconScale.xScale = 1.0
            iconScale.yScale = 1.0
            control.pressed(mouse)
            popupTips.hide()
        }

        onPositionChanged: {
            if (pressed) {
                if (control.draggable && mouse.source !== Qt.MouseEventSynthesizedByQt) {
                    drag.target = icon
                    icon.grabToImage(function(result) {
                        control.Drag.imageSource = result.url
                    })
                } else {
                    drag.target = null
                }
            }

            control.positionChanged()
        }

        onPressAndHold : control.pressAndHold(mouse)
        onReleased: {
            drag.target = null
            control.released()
        }

        onContainsMouseChanged: {
            if (containsMouse && control.popupText !== "") {
                popupTips.popupText = control.popupText

                if (Settings.direction === DockSettings.Left)
                    popupTips.position = Qt.point(root.width + LingmoUI.Units.largeSpacing,
                                                  control.mapToGlobal(0, 0).y + (control.height / 2 - popupTips.height / 2))
                else if (Settings.direction === DockSettings.Right)
                    popupTips.position = Qt.point(control.mapToGlobal(0, 0).x - popupTips.width - LingmoUI.Units.smallSpacing / 2,
                                                  control.mapToGlobal(0, 0).y + (control.height / 2 - popupTips.height / 2))
                else
                    popupTips.position = Qt.point(control.mapToGlobal(0, 0).x + (control.width / 2 - popupTips.width / 2),
                                                  control.mapToGlobal(0, 0).y - popupTips.height - LingmoUI.Units.smallSpacing)

                popupTips.show()
            } else {
                popupTips.hide()
            }
        }
    }

    Rectangle {
        id: activeRect

        property var leftX: 2
        property var leftY: (parent.height - height) / 2

        property var bottomX: (parent.width - width) / 2
        property var bottomY: icon.y + icon.height + activeRect.height / 2 - 1

        property var rightX: icon.x + icon.width + activeRect.width / 2 - 1
        property var rightY: (parent.height - height) / 2

        property var circleSize: isBottom ? control.height * 0.06 : control.width * 0.06
        property var activeLength: isBottom ? control.height * 0.5 : control.height * 0.5

        width: !isBottom ? circleSize : (isActive ? activeLength : circleSize)
        height: !isBottom ? (isActive ? activeLength : circleSize) : circleSize
        radius: !isBottom ? width / 2 : height / 2
        visible: enableActivateDot && !dragStarted
        color: LingmoUI.Theme.textColor

        x: isLeft ? leftX : isBottom ? bottomX : rightX
        y: isLeft ? leftY : isBottom ? bottomY : rightY

        Behavior on width {
            NumberAnimation {
                duration: isBottom ? 250 : 0
                easing.type: Easing.InOutSine
            }
        }

        Behavior on height {
            NumberAnimation {
                duration: !isBottom ? 250 : 0
                easing.type: Easing.InOutSine
            }
        }
    }
}
