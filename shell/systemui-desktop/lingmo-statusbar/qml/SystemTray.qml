import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import Lingmo.StatusBar 1.0
import LingmoUI 1.0 as LingmoUI

ListView {
    id: trayView

    orientation: Qt.Horizontal
    layoutDirection: Qt.RightToLeft
    interactive: false
    clip: true
    spacing: LingmoUI.Units.smallSpacing / 2

    property real itemWidth: rootItem.iconSize + LingmoUI.Units.largeSpacing

    Layout.fillHeight: true
    Layout.preferredWidth: (itemWidth + (count - 1) * LingmoUI.Units.smallSpacing) * count

    model: SystemTrayModel {
        id: trayModel
    }

    moveDisplaced: Transition {
        NumberAnimation {
            properties: "x, y"
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    delegate: StandardItem {
        id: _trayItem

        property bool darkMode: rootItem.darkMode
        property int dragItemIndex: index
        property bool dragStarted: false

        width: trayView.itemWidth
        height: ListView.view.height
        animationEnabled: true

        onDarkModeChanged: updateTimer.restart()

        Drag.active: _trayItem.mouseArea.drag.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.MoveAction
        Drag.hotSpot.x: iconItem.width / 2
        Drag.hotSpot.y: iconItem.height / 2

        Drag.onDragStarted:  {
            dragStarted = true
        }

        Drag.onDragFinished: {
            dragStarted = false
        }

        onPositionChanged: {
            if (_trayItem.mouseArea.pressed) {
                _trayItem.mouseArea.drag.target = iconItem
                iconItem.grabToImage(function(result) {
                    _trayItem.Drag.imageSource = result.url
                })
            } else {
                _trayItem.mouseArea.drag.target = null
            }
        }

        onReleased: {
            _trayItem.mouseArea.drag.target = null
        }

        DropArea {
            anchors.fill: parent
            enabled: true

            onEntered: {
                if (drag.source)
                    trayModel.move(drag.source.dragItemIndex,
                                   _trayItem.dragItemIndex)
            }
        }

        Timer {
            id: updateTimer
            interval: 10
            onTriggered: iconItem.updateIcon()
        }

        ColorOverlay {
            id: iconOverlay
            anchors.centerIn: parent
            width: rootItem.iconSize
            height: width
            source: iconItem
            color: rootItem.textColor
            opacity: rootItem.darkMode ? 1 : 0.7
            visible: model.canColorOverlay
        }

        LingmoUI.IconItem {
            id: iconItem
            anchors.centerIn: parent
            width: rootItem.iconSize
            height: width
            source: model.iconName ? model.iconName : model.icon
            antialiasing: true
            smooth: true
            visible: !dragStarted && !iconOverlay.visible
        }

        onClicked: {
            var pos = trayModel.popupPosition(_trayItem, mouse.x, mouse.y)

            if (mouse.button === Qt.LeftButton) {
                trayModel.leftButtonClick(model.id, pos.x, pos.y)
            } else if (mouse.button === Qt.RightButton) {
                trayModel.rightButtonClick(model.id, _trayItem, pos.x, pos.y)
            } else if (mouse.button === Qt.MiddleButton) {
                trayModel.middleButtonClick(model.id, pos.x, pos.y)
            }
        }

        popupText: model.toolTip ? model.toolTip : model.title
    }
}
