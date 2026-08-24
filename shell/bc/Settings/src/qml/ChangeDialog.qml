import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Lingmo.Settings 1.0
import LingmoUI 1.0 as LingmoUI

LingmoUI.Window {
    id: control

    width: 400
    height: 360

    maximumWidth: control.width
    minimumWidth: control.width

    maximumHeight: control.height
    minimumHeight: control.height

    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.FramelessWindowHint

    title: qsTr("Device")

    minimizeButtonVisible: false
    // maximizeButtonVisible: false
    visible: false

    // property var iconSource
    // property string name
    // property string description
    // property string link: "https://lingmo.org"
    // property var contentHeight: _mainLayout.implicitHeight + control.header.height * 2

    background.opacity: control.compositing ? 0.6 : 1.0

    LingmoUI.WindowBlur {
        view: control
        geometry: Qt.rect(control.x, control.y, control.width, control.height)
        windowRadius: control.windowRadius
        enabled: control.compositing
    }

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: if (active) { control.helper.startSystemMove(control) }
    }

    function clear() {
        deviceChange.clear()
    }

    HostNameChanger {
        id: hostnameChanger
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: mainPageComponent
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: control.header.height

        // Image {
        //     width: 64
        //     height: 64
        //     source: control.iconSource
        //     sourceSize: Qt.size(64, 64)
        //     Layout.alignment: Qt.AlignHCenter
        // }

        Rectangle {
            id: deviceItem

            anchors {
                verticalCenter: control.verticalCenter
                horizontalCenter: control.horizontalCenter
            }
            Layout.alignment: Qt.AlignHCenter
            width: 207
            height: 125
            color: LingmoUI.Theme.backgroundColor
            border.width: 5
            border.color: LingmoUI.Theme.textColor
            radius: LingmoUI.Theme.bigRadius

            Image {
                id: _image
                width: deviceItem.width - 5
                height: deviceItem.height - 5
                anchors {
                    verticalCenter: deviceItem.verticalCenter
                    horizontalCenter: deviceItem.horizontalCenter
                }
                // source: LingmoUI.Theme.darkMode ? "qrc:/images/MundoDark.jpeg" : "qrc:/images/MundoLight.jpeg"
                source: "file://" + wallpaper.path
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                mipmap: true
                cache: true
                smooth: true
                opacity: 1.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutCubic
                    }
                }

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: _image.width
                        height: _image.height
                        Rectangle {
                            anchors.fill: parent
                            radius: LingmoUI.Theme.bigRadius
                        }
                    }
                }
            }

            // Loader {
            //     id: bgLoader
            //     anchors.fill: parent
            //     sourceComponent: {
            //         if (background.backgroundType === 0)
            //             return wallpaperItem

            //         return colorItem
            //     }
            // }
        }

        Text {
            id: pcNv
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 5
            text: about.hostName
            color: LingmoUI.Theme.textColor
            font.pointSize: 9
        }

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        Text {
            id: wsa
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 15
            text: qsTr("Enter your new device name")
            color: LingmoUI.Theme.textColor
            font.pointSize: 10
        }

        TextField {
            id: deviceChange
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 5
            width: parent.width - 9
            placeholderText: qsTr("New device name")
            selectByMouse: true
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Button {
            id: btn
            enabled: deviceChange.text != ""
            // anchors.horizontalCenter: parent.horizontalCenter
            // Layout.right: _mainLayout.right
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 20
            flat: true
            text: qsTr("Ok")
            onClicked: {
                pcDialog.visible = false
                hostnameChanger.onHostNameChanged(deviceChange.text);
            }
        }

        // Label {
        //     text: control.name
        //     Layout.alignment: Qt.AlignHCenter
        //     font.pointSize: 14
        // }

        // Label {
        //     text: control.description
        //     Layout.alignment: Qt.AlignHCenter
        // }

        // Label {
        //     text: "<a href=\"%1\">%1</a>".arg(control.link)
        //     Layout.alignment: Qt.AlignHCenter
        //     linkColor: LingmoUI.Theme.highlightColor

        //     MouseArea {
        //         anchors.fill: parent
        //         cursorShape: Qt.PointingHandCursor
        //         onClicked: Qt.openUrlExternally(control.link)
        //     }
        // }

        Item {
            Layout.fillHeight: true
        }
    }
}
