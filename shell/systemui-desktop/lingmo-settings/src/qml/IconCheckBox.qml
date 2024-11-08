import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.12
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property var iconSpacing: LingmoUI.Units.smallSpacing * 0.8
    property alias source: icon.source
    property alias text: label.text
    property bool checked: false

    property var iconSize: 96

    signal clicked

    implicitHeight: mainLayout.implicitHeight
    implicitWidth: mainLayout.implicitWidth

    scale: 1.0

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        Rectangle {
            id: _box
            width: control.iconSize
            height: width
            color: "transparent"
            border.width: 3
            border.color: control.checked ? LingmoUI.Theme.highlightColor : "transparent"

            radius: LingmoUI.Theme.bigRadius + control.iconSpacing
            visible: true

            Image {
                id: icon
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing
                sourceSize: Qt.size(icon.width, icon.height)
                opacity: 1
                smooth: false

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: icon.width
                        height: icon.height

                        Rectangle {
                            anchors.fill: parent
                            radius: LingmoUI.Theme.bigRadius
                        }
                    }
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.OutSine
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: function() {
                        icon.opacity = 0.8
                    }
                    onExited: function() {
                        icon.opacity = 1.0
                    }
                }
            }
        }

        Label {
            id: label
            color: control.checked ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.textColor
            visible: label.text
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutSine
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()

        onPressedChanged: {
            control.scale = pressed ? 0.95 : 1.0
        }
    }
}
