import QtQuick 2.15
import QtQuick.Controls 2.15
import LingmoUI 1.0 as LingmoUI

Label {
    id: control

    property bool showSortIndicator: false
    property bool ascending: false
    property alias textAlignment: control.horizontalAlignment

    color: LingmoUI.Theme.textColor
    opacity: 0.7

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: control.clicked()
    }

    Image {
        anchors.left: parent.right
        anchors.leftMargin: LingmoUI.Units.smallSpacing
        anchors.verticalCenter: parent.verticalCenter
        visible: showSortIndicator
        source: "qrc:/images/go-down.svg"
        width: 12
        height: 12
        rotation: ascending ? 180 : 0

        Behavior on rotation {
            NumberAnimation {
                duration: 200
            }
        }
    }

    signal clicked
} 