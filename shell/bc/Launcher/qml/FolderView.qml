import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI

Rectangle {
    id: folderView
    anchors.fill: parent
    color: "#202020"
    visible: false

    property var sourceApps: []

    GridView {
        id: folderGrid
        anchors.fill: parent
        model: sourceApps
        cellWidth: 128
        cellHeight: 128
        delegate: GridItemDelegate {
            model: modelData
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: folderView.visible = false
    }
}
