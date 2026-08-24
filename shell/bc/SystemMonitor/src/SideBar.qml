import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    implicitWidth: 230

    property int itemRadiusV: 8

    property alias view: listView
    property alias model: listModel
    property alias currentIndex: listView.currentIndex

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.backgroundColor, 1.5)
                                     : Qt.darker(LingmoUI.Theme.backgroundColor, 1.05)
        opacity: rootWindow.compositing ? 0.3 : 0.4
    }

    ListModel {
        id: listModel

        ListElement {
            title: qsTr("Overview")
            name: "overview"
            iconSource: "overview.svg"
            page: "qrc:/Overview.qml"
        }

        ListElement {
            title: qsTr("Processes")
            name: "processes"
            iconSource: "processes.svg"
            page: "qrc:/Processes.qml"
        }

        ListElement {
            title: qsTr("Resources")
            name: "resources" 
            iconSource: "resources.svg"
            page: "qrc:/Resources.qml"
        }

        ListElement {
            title: qsTr("File Systems")
            name: "filesystem"
            iconSource: "filesystem.svg"
            page: "qrc:/FileSystem.qml"
        }

        ListElement {
            title: qsTr("Settings")
            name: "settings"
            iconSource: "settings.svg"
            page: "qrc:/Settings.qml"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        Label {
            text: rootWindow.title
            color: rootWindow.active ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor
            Layout.preferredHeight: rootWindow.header.height
            leftPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            rightPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            topPadding: LingmoUI.Units.smallSpacing
            bottomPadding: 0
            font.pixelSize: 14
        }

        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: listModel

            spacing: LingmoUI.Units.smallSpacing
            leftMargin: LingmoUI.Units.largeSpacing
            rightMargin: LingmoUI.Units.largeSpacing
            topMargin: 0
            bottomMargin: LingmoUI.Units.largeSpacing

            ScrollBar.vertical: ScrollBar {}

            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0
            highlightResizeDuration : 0
            highlight: Rectangle {
                radius: LingmoUI.Theme.mediumRadius
                color: LingmoUI.Theme.highlightColor
                smooth: true
            }

            delegate: Item {
                id: item
                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                height: LingmoUI.Units.fontMetrics.height + LingmoUI.Units.largeSpacing * 1.5

                property bool isCurrent: listView.currentIndex === index

                Rectangle {
                    anchors.fill: parent

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: listView.currentIndex = index
                    }

                    radius: LingmoUI.Theme.mediumRadius
                    color: mouseArea.containsMouse && !isCurrent ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                                           LingmoUI.Theme.textColor.g,
                                                                           LingmoUI.Theme.textColor.b,
                                                                   0.1) : "transparent"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.smallSpacing * 1.5

                    Rectangle {
                        id: iconRect
                        width: 24
                        height: 24
                        Layout.alignment: Qt.AlignVCenter
                        radius: 20
                        color: LingmoUI.Theme.highlightColor

                        Image {
                            id: icon
                            anchors.centerIn: parent
                            width: 16
                            height: width
                            source: "qrc:/images/" + model.iconSource
                            sourceSize: Qt.size(width, height)
                            Layout.alignment: Qt.AlignVCenter
                            antialiasing: false
                            smooth: false
                        }
                    }

                    Label {
                        id: itemTitle
                        text: model.title
                        color: isCurrent ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}