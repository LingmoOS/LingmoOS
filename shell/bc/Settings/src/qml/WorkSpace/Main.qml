import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.System 1.0 as System
import Lingmo.Settings 1.0
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Work Space")

    Workspace {
        id: workspace
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            Label {
                text: qsTr("Workspace Behavior")
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openKwinscreenedges()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Screen edge")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openDisplayeffect()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Display Effects Compositor")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openTaskSwitcher()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Task Switcher")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openFontmanager()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Font Manager")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openFileAssociations()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("File Associations")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            Label {
                text: qsTr("Global Search")
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openFilesearch()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("File Search")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openWebsearch()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Web search keywords")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            Label {
                text: qsTr("Hardware")
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    workspace.openKeyboard()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Keyboard")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }
        }
    }
}
