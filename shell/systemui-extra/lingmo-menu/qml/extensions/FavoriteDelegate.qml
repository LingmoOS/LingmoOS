import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import org.lingmo.menu.core 1.0
import org.lingmo.menu.extension 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import AppControls2 1.0 as AppControls2

LingmoUIItems.StyleBackground {
    id: iconItem
    property bool hold: false
    property alias delegateLayout: itemLayout
    property alias mergePrompt: mergePrompt
    property bool isFullScreen: false
    radius: 8
    useStyleTransparency: false
    paletteRole: Platform.Theme.Text
    alpha: hold ? 0 : control.containsPress ? 0.15 : control.containsMouse ? 0.08 : 0

    property int visualIndex
    Binding { target: itemLoader; property: "visualIndex"; value: visualIndex }

    // 合并提示框 52*52
    LingmoUIItems.StyleBackground {
        id: mergePrompt
        height: width
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        paletteRole: Platform.Theme.Text
        useStyleTransparency: false
        alpha: ((delegateDropArea.containsDrag || moveDropArea.containsDrag) && favoriteView.dragTypeIsMerge) ? 0.15 : 0
        z: -1
    }

    DropArea {
        id: delegateDropArea
        anchors.fill: parent

        // drag.source [itemLoader]
        onEntered: {
            // 拖拽对象为folder 或 左侧列表应用时
            if (!drag.source.sourceId) {
                drag.accepted = false;
                return;
            }
            if (!drag.source.isFolder && drag.source.sourceId !== model.id) {
                delegateDropTimer.running = true;
            }
        }
        onExited: {
            if (!drag.source.sourceId) {
                return;
            }

            delegateDropTimer.running = false;
        }

        DropArea {
            id: moveDropArea
            anchors.fill: parent
            anchors.margins: delegateDropArea.width * 0.2

            onEntered: {
                // 拖拽对象为folder 或 左侧列表应用时
                if (!drag.source.sourceId) {
                    drag.accepted = false;
                    return;
                }
                delegateDropTimer.running = false;
                if (delegateDropTimer.timeOutCount < 1) {
                    favoriteView.dragTypeIsMerge = false;
                    favoriteView.viewModel.items.move(drag.source.visualIndex, iconItem.visualIndex);
                }
            }
        }
    }

    Timer {
        id: delegateDropTimer
        property int timeOutCount: 0
        interval: 300
        repeat: true

        onTriggered: {
            ++timeOutCount;
            if (timeOutCount == 1) {
                favoriteView.mergeToAppId = model.id;
                favoriteView.isMergeToFolder = (model.type === DataType.Folder);
                favoriteView.dragTypeIsMerge = true;
            }
        }
        onRunningChanged: timeOutCount = 0
    }

    MouseArea {
        id: control
        anchors.fill: parent
        hoverEnabled: true
        pressAndHoldInterval: 300
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (isFullScreen) {
                fullScreenUI.focus =false;
            } else {
                normalUI.focus = false;
            }

            itemLoader.item.itemClicked(mouse.button);
        }
        onPressAndHold: {
            if (mouse.button === Qt.LeftButton) {
                if (isFullScreen) {
                    fullScreenUI.focus =false;
                } else {
                    normalUI.focus = false;
                }
                drag.target = itemLoader;
                iconItem.hold = true;
                favoriteView.exchangedStartIndex = itemLoader.visualIndex;
                itemLoader.sourceId = model.id;
            }
        }
        onReleased: {
            drag.target = null;
        }
    }

    ColumnLayout {
        id: itemLayout
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.bottomMargin: 14
        spacing: 4

        ToolTip.visible: iconText.truncated && control.containsMouse
        ToolTip.text: model.name
        ToolTip.delay: 500

        Item {
            id: loaderBase
            Layout.preferredWidth: iconItem.width * 0.6
            Layout.preferredHeight: width
            Layout.alignment: Qt.AlignHCenter

            Loader {
                id: itemLoader
                width: loaderBase.width
                height: loaderBase.height
                x: 0; y: 0
                Drag.active: control.drag.active
                Drag.source: itemLoader
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2

                Drag.onActiveChanged: {
                    if (Drag.active) {
                        itemLoader.parent = favoriteView;
                    } else {
                        if (favoriteView.dragTypeIsMerge
                                && (model.id !== favoriteView.mergeToAppId)
                                && (model.type === DataType.Normal)) {
                            iconItem.hold = false;

                        } else {
                            iconResetAnimation.start();
                        }
                    }
                }

                ParallelAnimation {
                    id: iconResetAnimation
                    NumberAnimation {
                        target: itemLoader
                        property: "x"
                        to: container.x + loaderBase.x
                        easing.type: Easing.OutQuad
                        duration: 300
                    }
                    NumberAnimation {
                        target: itemLoader
                        property: "y"
                        to: (container.y + 8) - favoriteView.contentY
                        easing.type: Easing.OutQuad
                        duration: 300
                    }

                    onFinished: {
                        iconItem.hold = false;
                        itemLoader.parent = loaderBase;
                        itemLoader.x = 0; itemLoader.y = 0;
                    }
                }

                property int visualIndex: 0
                property string sourceId: ""
                property bool isFolder: model.type === DataType.Folder
                property string icon: model.icon
                property bool isFavorite: true
                sourceComponent: {
                    if (type === DataType.Normal) {
                        return appIconComponent;
                    }
                    if (type === DataType.Folder) {
                        return folderIconComponent;
                    }
                }
            }
        }


        LingmoUIItems.StyleText {
            id: iconText
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            Layout.maximumHeight: contentHeight

            text: name
            elide: Text.ElideRight
            paletteRole: Platform.Theme.Text
            horizontalAlignment: Text.AlignHCenter
            opacity: !itemLoader.Drag.active

            Behavior on opacity {
                NumberAnimation { duration: 150}
            }
        }
    }

    Component {
        id: appIconComponent
        LingmoUIItems.Icon {
            id: iconImage
            source: icon

            function itemClicked(mouseButton) {
                if (mouseButton === Qt.RightButton) {
                    favoriteModel.openMenu(index)
                } else {
//                    var data = {"id": model.id};
//                    send(data);
                    appManager.launchApp(id);
                }
            }
        }
    }

    Component {
        id: folderIconComponent
        Item {
            states: [
                State {
                    name: "iconHide"
                    PropertyChanges { target: folderIcon; opacity: 0 }
                },
                State {
                    name: "iconShow"
                    PropertyChanges { target: folderIcon; opacity: 1 }
                }
            ]

            transitions: [
                Transition {
                    to: "iconShow"
                    NumberAnimation { duration: 300; easing.type: Easing.InQuint }
                }
            ]

            AppControls2.FolderIcon {
                id: folderIcon
                height: width
                anchors.centerIn: parent
                alpha: 0.10
                icons: icon
                columns: rows

                width: isFullScreen ? 84: 40
                rows: isFullScreen ? 4 : 2
                spacing: isFullScreen ? 4 : 2
                padding: isFullScreen ? 8 : 2
                radius: isFullScreen ? Platform.Theme.maxRadius : Platform.Theme.normalRadius
            }
            function itemClicked(mouseButton) {
                if (mouseButton === Qt.RightButton) {
                    favoriteModel.openMenu(index);
                } else {
                    var x = mapToGlobal(folderIcon.x, folderIcon.y).x;
                    var y = mapToGlobal(folderIcon.x, folderIcon.y).y
                    openFolderSignal(id, name, x, y);
                    // 执行隐藏动画，并且当前图标消失且鼠标区域不可用
                    state = "iconHide";
                    control.enabled = false;
                    control.hoverEnabled = false;
                }
            }
        }
    }

    // folderFunction
    function resetOpacity() {
        itemLoader.item.state = "iconShow";
        control.enabled = true;
        control.hoverEnabled = true;
    }

    onHoldChanged: {
        if (hold) {
            favoriteView.interactive = false;
        } else {
            favoriteView.interactive = contentHeight > favoriteView.parent.height;
            if (favoriteView.dragTypeIsMerge && (model.id !== favoriteView.mergeToAppId) && (model.type === DataType.Normal)) {
                if (favoriteView.isMergeToFolder) {
                    favoriteModel.addAppToFolder(model.id, favoriteView.mergeToAppId);
                } else {
                    favoriteModel.addAppsToNewFolder(model.id, favoriteView.mergeToAppId);
                }

            } else if (favoriteView.exchangedStartIndex !== itemLoader.visualIndex) {
                favoriteModel.exchangedAppsOrder(favoriteView.exchangedStartIndex, itemLoader.visualIndex);
            }
        }
    }
}
