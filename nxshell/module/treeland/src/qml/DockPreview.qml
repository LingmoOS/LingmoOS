import QtQuick
import Waylib.Server
import TreeLand.Utils
import QtQuick.Controls

Item {
    id: root
    clip: true

    signal stopped
    signal entered(var relativeSurface)
    signal exited(var relativeSurface)
    signal surfaceActivated(Item surface)

    visible: false

    property bool previewing: context.item?.visible // TODO: use loader.active

    property var target
    property var pos
    property int direction: 0

    property int checkExited: 0
    property bool isEntered: false

    function show(surfaces, target, pos, direction) {
        filterModel.desiredSurfaces = surfaces
        root.pos = pos;
        root.direction = direction;
        root.target = target;
        visible = true;
    }

    function close() {
        visible = false;
        filterModel.desiredSurfaces = []
        stopped();
    }

    Loader {
        id: context
    }

    Component {
        id: contextComponent

        WindowsSwitcherPreview {
        }
    }

    FilterProxyModel {
        id: filterModel
        sourceModel: QmlHelper.workspaceManager.allSurfaces
        filterAcceptsRow: (data) => {
            return desiredSurfaces.some(surface => data.item.shellSurface.surface === surface)
        }
        property var desiredSurfaces: []
        onDesiredSurfacesChanged: invalidate()
    }

    Timer {
        id: exitedTimer
        interval: 100
        onTriggered: {
            root.exited(root.target.item.shellSurface.surface);
            context.item.stop();
            root.close()
        }
    }

    Rectangle {
        width: box.width
        height: box.height
        x: box.x
        y: box.y

        radius: 10
        opacity: 0.4
    }

    Item {
        id: box

        x: root.direction === 0 ? root.target.shell.x + root.pos.x - width / 2 :
           root.direction === 1 ? root.target.shell.x + root.pos.x - width :
           root.direction === 2 ? root.target.shell.x + root.pos.x - width / 2 :
           root.direction === 3 ? root.target.shell.x + root.target.shell.width : 0

        y: root.direction === 0 ? root.target.shell.y + root.target.shell.height :
           root.direction === 1 ? root.target.shell.y + root.pos.y - height / 2 :
           root.direction === 2 ? root.target.shell.y - height :
           root.direction === 3 ? root.target.shell.y + root.pos.y - height / 2 : 0

        width: listView.width
        height: top.height + listView.height

        HoverHandler {
            acceptedDevices: PointerDevice.AllDevices // WTF: why need allDevices?
            cursorShape: Qt.PointingHandCursor
            onHoveredChanged: {
                if (hovered) {
                    root.entered(root.target.item.shellSurface.surface);
                }
                else {
                    exitedTimer.start();
                }
            }
        }

        Item {
            id: top
            width: title.width + closeAllBtn.width
            height: title.height
            anchors.top: parent.top
            anchors.left: parent.left

            Text {
                id: title
                width: listView.width - closeAllBtn.width
                clip: true
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 15
                text: "Title"
            }

            Button {
                id: closeAllBtn
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 5
                anchors.topMargin: 5
                text: "X"
                onClicked: {
                    for (let i = 0; i < filterModel.count; i++) {
                        const item = filterModel.get(i).item
                        Helper.closeSurface(item.shellSurface.surface)
                    }
                    exitedTimer.start();
                    closeAllBtn.visible = false // WTF: why this button cannot hide when root is hide.
                }
            }
        }

        ListView {
            id: listView
            anchors.top: top.bottom
            anchors.left: parent.left
            height: root.direction % 2 ? listView.count * 180 : 180
            width: root.direction % 2 ? 180 : listView.count * 180
            orientation: root.direction % 2 ? ListView.Vertical : ListView.Horizontal
            model: filterModel
            delegate: Item {
                required property Item item
                required property var wrapper
                property Item surfaceItem: item
                width: 180
                height: 180
                clip: true
                visible: true

                ShaderEffectSource {
                    id: effect
                    anchors.centerIn: parent
                    width: parent.width - 20
                    height: Math.min(surfaceItem.height * width / surfaceItem.width, width)
                    live: true
                    hideSource: false
                    smooth: true
                    sourceItem: surfaceItem
                    HoverHandler {
                        acceptedDevices: PointerDevice.AllDevices
                        cursorShape: Qt.PointingHandCursor
                        onHoveredChanged: {
                            if (hovered) {
                                root.isEntered = true;
                                closeBtn.visible = true
                                closeAllBtn.visible = false
                                title.text = surfaceItem.shellSurface.title

                                context.parent = root.parent;
                                context.anchors.fill = root;
                                context.sourceComponent = contextComponent;
                                context.item.start(surfaceItem);
                            }
                            else {
                                title.text = "Title"
                                closeBtn.visible = false
                                closeAllBtn.visible = true
                                context.item.stop()
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            surfaceActivated(wrapper);
                            exitedTimer.start();
                        }
                    }

                    Button {
                        id: closeBtn
                        visible: false
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.rightMargin: 5
                        anchors.topMargin: 5
                        text: "X"
                        onClicked: {
                            Helper.closeSurface(surfaceItem.shellSurface.surface);
                        }
                    }
                }
            }
        }
    }
}
