import QtQuick 2.0
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

// https://techbase.kde.org/Development/Tutorials/KWin/WindowSwitcher
KWin.Switcher {
    id: tabBox
    currentIndex: itemsView.currentIndex

    Window {
        id: dialog
        visible: tabBox.visible
        // FramelessWindowHint is needed under wayland platform
        flags: Qt.BypassWindowManagerHint | Qt.FramelessWindowHint
        color: "transparent"

        //NOTE: this is the *current* screen, not the *primary* screen
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        onVisibleChanged: {
            if (visible) {
                dialogMainItem.calculateColumnCount();
                dialogMainItem.composited = tabBox.compositing;
            } else {
                itemsView.highCount = 0;
            }
        }

        Component.onCompleted: {
            if (typeof(dde) != 'undefined' && "enableDxcb" in dde) {
                dde.enableDxcb(dialogMainItem)
            }
        }

        width: dialogMainItem.width
        height: dialogMainItem.height

        //延时刷新定时器,用于wayland平台下,用户alt+tab 和 alt+ ~ 切换时候
        //导致tabBox 被截断的问题,详情:https://pms.uniontech.com/zentao/bug-view-84001.html
        Timer {
            id: updateTimer
            interval:1
            triggeredOnStart: false
            onTriggered: {
               //重新调整窗口的宽高为内容的宽高,确保窗口能再次刷新
               dialog.width  = dialogMainItem.width
               dialog.height = dialogMainItem.height
            }
        }

        function dealyUpdate() {
            //若定时器已经生效,则停止计时器
            if (updateTimer.running) {
                updateTimer.stop();
            }

            //重新启动
            updateTimer.start()
        }

        Rectangle {
            id: dialogMainItem

            QtObject {
                id: constants

                readonly property int minItemBox: 128
                readonly property int defaultBoxMargin: 32
                readonly property int popupPadding: 70

                readonly property int columnSpacing: 20
                readonly property int rowSpacing: 20
                readonly property int minItemsEachRow: 7
                readonly property int maxRows: 2
            }

            anchors.margins: 0
            color: "#4cffffff"
            radius: tabBox.windowRadius
            antialiasing: true
            border.width: 1
            border.color: "#19000000"

            property bool composited: true

            readonly property int maxWidth: tabBox.screenGeometry.width - constants.popupPadding * 2
            property int maxHeight: tabBox.screenGeometry.height * 0.7

            property real screenFactor: tabBox.screenGeometry.width / tabBox.screenGeometry.height
            property int maxGridColumnsByWidth: Math.floor(maxWidth / itemsView.cellWidth)

            property int gridColumns: maxGridColumnsByWidth
            property int gridRows: 1

            property int optimalWidth: itemsView.cellWidth * gridColumns
            property int optimalHeight: itemsView.cellHeight * gridRows

            property bool canStretchX: false
            property bool canStretchY: false
            width: Math.min(Math.max(itemsView.cellWidth, optimalWidth), maxWidth)
            height: Math.min(Math.max(itemsView.cellHeight, optimalHeight), maxHeight)

            clip: true

            Accessible.role: Accessible.Window
            Accessible.name: "tabbox_area"
            Accessible.description: "alt + tab switcher area"

            function calculateColumnCount() {
                var count = itemsView.count
                var item_need_scale = false
                var spacing = constants.defaultBoxMargin * 2 + 4
                var item_width = constants.minItemBox + constants.columnSpacing
                var maxWidth = tabBox.screenGeometry.width - constants.popupPadding * 2

                var max_items_each_row = Math.floor((maxWidth - spacing) / item_width);
                if (max_items_each_row < constants.minItemsEachRow && count > max_items_each_row) {
                    item_need_scale = true;
                    max_items_each_row = Math.min(count, constants.minItemsEachRow);
                }

                if (max_items_each_row * constants.maxRows < count) {
                    max_items_each_row = Math.floor(count / constants.maxRows);
                    item_need_scale = true;
                }

                if (item_need_scale) {
                    item_width = maxWidth / max_items_each_row
                }

                max_items_each_row = Math.floor((maxWidth - spacing) / Math.floor(item_width));

                gridColumns = Math.min(max_items_each_row, count);
                gridRows = Math.ceil(count / max_items_each_row);
                if (gridRows == 0) gridRows = 1;

                optimalWidth = item_width * gridColumns + spacing
                optimalHeight = item_width * gridRows + spacing

                itemsView.thumbnailWidth = item_width;
                itemsView.thumbnailHeight = item_width;

                tabBox.model.setViewColumnCount(max_items_each_row);

                dialog.dealyUpdate(); //当列数发生变化的时候,延时刷新

                //console.log('------------------ optimalHeight: ' + optimalHeight +
                    //', optimalWidth: ' + optimalWidth +
                    //', max_items_each_row: ' + max_items_each_row +
                    //', gridColumns: ' + gridColumns +
                    //', item width: ' + item_width + ', count: ' + count +
                    //', need scale: ' + item_need_scale +
                    //', maxWidth: ' + maxWidth);
            }

            property bool mouseEnabled: false
            MouseArea {
                id: mouseDetector
                anchors.fill: parent
                hoverEnabled: true
                onPositionChanged: dialogMainItem.mouseEnabled = true

                Accessible.role: Accessible.Button
                Accessible.name: "Ma_tabbox_mouseDetector"
                Accessible.description: "tabbox_mouseDetector"
                Accessible.onPressAction: pressed()
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 0
                color: tabBox.windowColor
                radius: tabBox.windowRadius
                antialiasing: true
                border.width: 1
                border.color: "#19000000"

                Component {
                    id: highlight
                    Rectangle {
                        width: itemsView.cellWidth
                        height: itemsView.cellHeight

                        //color: "#99000000"
                        color: tabBox.winHoverColor
                        radius: tabBox.winHoverRadius

                        x: itemsView.currentItem ? itemsView.currentItem.x : 0
                        y: itemsView.currentItem ? itemsView.currentItem.y : 0

                        Behavior on x { SmoothedAnimation { easing.type: Easing.InOutCubic; duration: 80 } }
                        Behavior on y { SmoothedAnimation { easing.type: Easing.InOutCubic; duration: 80 } }

                        Accessible.role: Accessible.Indicator
                        Accessible.name: "Rect_tabbox_Indicator_"+itemsView.currentIndex
                        Accessible.description: "tabbox_Indicator_currentIndex_currentWindowId"
                    }
                }

                Component {
                    id: desktopItem

                    Rectangle {
                        anchors.margins: constants.columnSpacing / 2
                        anchors.fill: parent
                        color: "transparent"

                        // PlasmaCore.WindowThumbnail has problem to correctly render desktop thumb
                        // so we use KWin.ThumbnailItem
                        /*
                        PlasmaCore.WindowThumbnail {
                            winId: modelWId
                            anchors.fill: parent
                        }
                        */
                        KWin.ThumbnailItem {
                            anchors.fill: parent
                            wId: modelWId
                        }

                        Accessible.role: Accessible.Graphic
                        Accessible.name: "Rect_tabbox_desktopImage_" + modelWId
                        Accessible.description: "tabbox_desktopImage_desktopImageId"
                    }

                }

                Component {
                    id: windowItem

                    Rectangle {
                        anchors.margins: constants.columnSpacing / 2
                        anchors.fill: parent
                        color: "transparent"

                        QIconItem {
                            id: iconItem
                            //FIXME: this is not the icon we want, seems to be a bug of kwin
                            icon: modelIcon
                            clip: true
                            anchors.fill: parent
                            smooth: true
                        }

                        // shadow for icon
                    /*    DropShadow {
                            anchors.fill: iconItem
                            horizontalOffset: 0
                            verticalOffset: 8
                            radius: 8.0
                            samples: 17
                            color: "#32000000"
                            source: iconItem
                        }*/

                        Accessible.role: Accessible.Graphic
                        Accessible.name: !isCloseable && (modelIndex+1) == itemsView.count ?
                                             "Rect_tabbox_desktopImage_" + modelWId : "Rect_tabbox_windowImage_" + modelWId
                        Accessible.description: "tabbox_windowImage_windowImageId"
                    }
                }

                GridView {
                    id: itemsView
                    model: tabBox.model
                    // interactive: false // Disable drag to scroll

                    anchors.fill: parent
                    anchors.margins: constants.defaultBoxMargin

                    property int thumbnailWidth: constants.minItemBox
                    property int thumbnailHeight: constants.minItemBox

                    cellWidth: thumbnailWidth
                    cellHeight: thumbnailHeight

                    highlight: highlight
                    highlightFollowsCurrentItem: false

                    // allow expansion on increasing count
                    property int highCount: 0
                    onCountChanged: {
                        if (highCount != count) {
                            dialogMainItem.calculateColumnCount();
                            highCount = count;
                        }
                    }


                    delegate: Item {
                        z: 1
                        width: itemsView.cellWidth
                        height: itemsView.cellHeight

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                parent.select()
                            }

                            Accessible.role: Accessible.Button
                            Accessible.name: !closeable && (index+1) == itemsView.count ?
                                                 "Ma_tabbox_desktopItem_" + windowId : "Ma_tabbox_windowItem_" + windowId
                            Accessible.description: "tabbox_window/desktopItem_winId"
                            Accessible.onPressAction: pressed()
                        }

                        function select() {
                            itemsView.currentIndex = index;
                            itemsView.currentIndexChanged(itemsView.currentIndex);
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"

                            Loader {
                                anchors.fill: parent

                                property int modelIndex: index
                                property variant modelWId: windowId
                                property variant modelIcon: model.icon
                                property bool isCloseable: closeable
                                sourceComponent: {
                                    if (index < 0) {
                                        return undefined
                                    }

                                    if (!dialogMainItem.composited) return windowItem;
                                    return !isCloseable && (index+1) == itemsView.count ? desktopItem: windowItem
                                }
                            }

                        }
                    } // GridView.delegate

                    Connections {
                        target: tabBox

                        onCurrentIndexChanged: {
                            itemsView.currentIndex = tabBox.currentIndex
                        }
                    }

                    // keyNavigationEnabled: true // Requires: Qt 5.7 and QtQuick 2.? (2.7 didn't work).
                    // keyNavigationWraps: true // Requires: Qt 5.7 and QtQuick 2.? (2.7 didn't work).

                } // GridView
            } // Dialog.mainItem
        }
    } // Dialog
}
