import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI
import "./components" as Components

ItemPage {
    id: fileSystem

    headerTitle: qsTr("File Systems")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        Components.CardItem {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: fsListView
                anchors.fill: parent
                anchors.margins: 1
                clip: true
                model: systemInfo.diskList
                spacing: 1

                header: Rectangle {
                    width: parent.width
                    height: 36
                    color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1) 
                                                  : Qt.rgba(0, 0, 0, 0.05)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: LingmoUI.Units.largeSpacing
                        anchors.rightMargin: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Components.HeaderLabel {
                            text: qsTr("Device")
                            Layout.preferredWidth: parent.width * 0.2
                        }

                        Components.HeaderLabel {
                            text: qsTr("Mount Point")
                            Layout.preferredWidth: parent.width * 0.2
                        }

                        Components.HeaderLabel {
                            text: qsTr("Type")
                            Layout.preferredWidth: parent.width * 0.1
                        }

                        Components.HeaderLabel {
                            text: qsTr("Total")
                            Layout.preferredWidth: parent.width * 0.1
                        }

                        Components.HeaderLabel {
                            text: qsTr("Free")
                            Layout.preferredWidth: parent.width * 0.1
                        }

                        Components.HeaderLabel {
                            text: qsTr("Used")
                            Layout.fillWidth: true
                        }
                    }
                }

                delegate: Rectangle {
                    width: parent.width
                    height: 48  // 增加行高以提高可读性
                    color: index % 2 === 0 ? "transparent"
                                         : (LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.05)
                                                                   : Qt.rgba(0, 0, 0, 0.025))

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered: parent.color = LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                                         : Qt.rgba(0, 0, 0, 0.05)
                        onExited: parent.color = index % 2 === 0 ? "transparent"
                                                               : (LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.05)
                                                                                        : Qt.rgba(0, 0, 0, 0.025))
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: LingmoUI.Units.largeSpacing
                        anchors.rightMargin: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: modelData.device
                            Layout.preferredWidth: parent.width * 0.2
                            elide: Text.ElideRight
                        }

                        Label {
                            text: modelData.mountPoint
                            Layout.preferredWidth: parent.width * 0.2
                            elide: Text.ElideRight
                        }

                        Label {
                            text: modelData.type
                            Layout.preferredWidth: parent.width * 0.1
                            elide: Text.ElideRight
                        }

                        Label {
                            text: modelData.total
                            Layout.preferredWidth: parent.width * 0.1
                            elide: Text.ElideRight
                        }

                        Label {
                            text: modelData.free
                            Layout.preferredWidth: parent.width * 0.1
                            elide: Text.ElideRight
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: LingmoUI.Units.smallSpacing

                                Components.ProgressBar {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter
                                    value: {
                                        let percentage = modelData.usedPercentage
                                        if (typeof percentage === 'string') {
                                            percentage = parseFloat(percentage)
                                        }
                                        return Math.min(Math.max(percentage / 1, 0), 100)
                                    }
                                    height: 6
                                }

                                Label {
                                    text: {
                                        let percentage = modelData.usedPercentage
                                        if (typeof percentage === 'string') {
                                            percentage = parseFloat(percentage)
                                        }
                                        return percentage.toFixed(1) + "%"
                                    }
                                    font.pointSize: 8
                                    opacity: 0.6
                                }
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}

                // 添加空状态提示
                Item {
                    anchors.centerIn: parent
                    visible: fsListView.count === 0
                    width: parent.width
                    height: emptyLabel.height

                    Label {
                        id: emptyLabel
                        anchors.centerIn: parent
                        text: qsTr("No file systems found")
                        opacity: 0.5
                    }
                }
            }
        }
    }
} 