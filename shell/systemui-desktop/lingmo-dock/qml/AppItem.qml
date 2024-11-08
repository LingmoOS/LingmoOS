import QtQuick 2.12
import QtQuick.Controls 2.12
import Lingmo.Dock 1.0
import LingmoUI 1.0 as LingmoUI

DockItem {
    id: appItem

    property var windowCount: model.windowCount
    property var dragSource: null

    iconName: model.iconName ? model.iconName : "application-x-desktop"
    isActive: model.isActive
    popupText: model.visibleName
    enableActivateDot: windowCount !== 0
    draggable: !model.fixed
    dragItemIndex: index

    onXChanged: {
        if (windowCount > 0)
            updateGeometry()
    }

    onYChanged: {
        if (windowCount > 0)
            updateGeometry()
    }

    onWindowCountChanged: {
        if (windowCount > 0)
            updateGeometry()
    }

    onPositionChanged: updateGeometry()
    onPressed: updateGeometry()
    onRightClicked: if (model.appId !== "lingmo-launcher") contextMenu.show()

    onClicked: {
        if (mouse.button === Qt.LeftButton)
            appModel.clicked(model.appId)
        else if (mouse.button === Qt.MiddleButton)
            appModel.openNewInstance(model.appId)
    }

    dropArea.onEntered: {
        appItem.dragSource = drag.source
        dropTimer.restart()
    }

    dropArea.onExited: {
        appItem.dragSource = null
        dropTimer.stop()
    }

    dropArea.onDropped: {
        appModel.save()
        updateGeometry()
    }

    Timer {
        id: dropTimer
        interval: 300
        onTriggered: {
            if (appItem.dragSource)
                appModel.move(appItem.dragSource.dragItemIndex,
                              appItem.dragItemIndex)
            else
                appModel.raiseWindow(model.appId)
        }
    }

    LingmoUI.DesktopMenu {
        id: contextMenu

        MenuItem {
            text: qsTr("Open")
            visible: windowCount === 0
            onTriggered: appModel.openNewInstance(model.appId)
        }

        MenuItem {
            text: model.visibleName
            visible: windowCount > 0 && model.visibleName
            onTriggered: appModel.openNewInstance(model.appId)
        }

        MenuItem {
            text: model.isPinned ? qsTr("Unpin") : qsTr("Pin")
            visible: model.desktopFile !== ""
            onTriggered: {
                model.isPinned ? appModel.unPin(model.appId) : appModel.pin(model.appId)
            }
        }

        MenuItem {
            visible: windowCount !== 0
            text: windowCount === 1 ? qsTr("Close window")
                                    : qsTr("Close %1 windows").arg(windowCount)
            onTriggered: appModel.closeAllByAppId(model.appId)
        }
    }


    function updateGeometry() {
        if (model.fixed)
            return

        appModel.updateGeometries(model.appId, Qt.rect(appItem.mapToGlobal(0, 0).x,
                                                       appItem.mapToGlobal(0, 0).y,
                                                       appItem.width, appItem.height))
    }
}
