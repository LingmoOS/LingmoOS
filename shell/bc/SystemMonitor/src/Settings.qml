import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI
import "./components" as Components
import Lingmo.SystemMonitor 1.0

ItemPage {
    id: settings
    headerTitle: qsTr("Settings")

    property int defaultInterval: 2000

    SettingsManager {
        id: settingsManager
    }

    Component.onCompleted: {
        updateIntervalSpinBox.value = settingsManager.updateInterval
        graphPointsSpinBox.value = settingsManager.graphPoints
        
        var sortFields = ["cpu", "memory", "name", "user"]
        sortByBox.currentIndex = sortFields.indexOf(settingsManager.defaultSortBy)
        sortOrderBox.currentIndex = settingsManager.sortAscending ? 1 : 0
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: LingmoUI.Units.largeSpacing
            
            Item {
                width: parent.width
                height: LingmoUI.Units.largeSpacing
            }

            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: generalLayout.implicitHeight + LingmoUI.Units.largeSpacing * 6
                title: qsTr("General")

                ColumnLayout {
                    id: generalLayout
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing

                    RowLayout {
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("Update Interval:")
                            Layout.preferredWidth: 120
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.7
                        }

                        SpinBox {
                            id: updateIntervalSpinBox
                            from: 1000
                            to: 10000
                            stepSize: 500
                            value: defaultInterval
                            Layout.preferredHeight: 36
                            onValueChanged: {
                                systemInfo.updateInterval = value
                                settingsManager.updateInterval = value
                            }

                            textFromValue: function(value) {
                                return (value / 1000).toFixed(1)
                            }
                            valueFromText: function(text) {
                                return text * 1000
                            }

                            contentItem: TextInput {
                                text: updateIntervalSpinBox.textFromValue(updateIntervalSpinBox.value, updateIntervalSpinBox.locale)
                                font: updateIntervalSpinBox.font
                                color: LingmoUI.Theme.textColor
                                selectionColor: LingmoUI.Theme.highlightColor
                                selectedTextColor: LingmoUI.Theme.highlightedTextColor
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !updateIntervalSpinBox.editable
                                validator: updateIntervalSpinBox.validator
                                selectByMouse: true
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 36
                                color: LingmoUI.Theme.backgroundColor
                                radius: LingmoUI.Theme.smallRadius
                            }

                            down.indicator: Rectangle {
                                x: 0
                                width: 28
                                height: parent.height
                                color: updateIntervalSpinBox.down.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       updateIntervalSpinBox.down.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "−"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }

                            up.indicator: Rectangle {
                                x: parent.width - width
                                width: 28
                                height: parent.height
                                color: updateIntervalSpinBox.up.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       updateIntervalSpinBox.up.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "+"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }
                        }

                        Label {
                            text: qsTr("seconds")
                            opacity: 0.7
                        }

                        Button {
                            text: qsTr("Reset")
                            icon.name: "edit-undo"
                            onClicked: updateIntervalSpinBox.value = defaultInterval

                            background: Rectangle {
                                radius: LingmoUI.Theme.smallRadius
                                color: parent.pressed ? Qt.rgba(0, 0, 0, 0.2) :
                                                       parent.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("Refresh Rate:")
                            Layout.preferredWidth: 120
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.7
                        }

                        SpinBox {
                            id: refreshRateSpinBox
                            from: 100  // 0.1秒
                            to: 10000  // 10秒
                            stepSize: 100
                            value: settingsManager.updateInterval
                            Layout.preferredHeight: 36
                            onValueChanged: {
                                systemInfo.updateInterval = value
                                settingsManager.updateInterval = value
                            }

                            textFromValue: function(value) {
                                return (value / 1000).toFixed(1)
                            }
                            valueFromText: function(text) {
                                return text * 1000
                            }

                            contentItem: TextInput {
                                text: refreshRateSpinBox.textFromValue(refreshRateSpinBox.value, refreshRateSpinBox.locale)
                                font: refreshRateSpinBox.font
                                color: LingmoUI.Theme.textColor
                                selectionColor: LingmoUI.Theme.highlightColor
                                selectedTextColor: LingmoUI.Theme.highlightedTextColor
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !refreshRateSpinBox.editable
                                validator: refreshRateSpinBox.validator
                                selectByMouse: true
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 36
                                color: LingmoUI.Theme.backgroundColor
                                radius: LingmoUI.Theme.smallRadius
                            }

                            down.indicator: Rectangle {
                                x: 0
                                width: 28
                                height: parent.height
                                color: refreshRateSpinBox.down.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       refreshRateSpinBox.down.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "−"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }

                            up.indicator: Rectangle {
                                x: parent.width - width
                                width: 28
                                height: parent.height
                                color: refreshRateSpinBox.up.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       refreshRateSpinBox.up.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "+"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }
                        }

                        Label {
                            text: qsTr("seconds")
                            opacity: 0.7
                        }

                        Button {
                            text: qsTr("Reset")
                            icon.name: "edit-undo"
                            onClicked: refreshRateSpinBox.value = 1000  // 重置为1秒

                            background: Rectangle {
                                radius: LingmoUI.Theme.smallRadius
                                color: parent.pressed ? Qt.rgba(0, 0, 0, 0.2) :
                                                      parent.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("Graph Points:")
                            Layout.preferredWidth: 120
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.7
                        }

                        SpinBox {
                            id: graphPointsSpinBox
                            Layout.preferredWidth: 120
                            from: 30
                            to: 200
                            stepSize: 10
                            value: settings.graphPoints
                            editable: true

                            contentItem: TextInput {
                                text: graphPointsSpinBox.textFromValue(graphPointsSpinBox.value, graphPointsSpinBox.locale)
                                font: graphPointsSpinBox.font
                                color: LingmoUI.Theme.textColor
                                selectionColor: LingmoUI.Theme.highlightColor
                                selectedTextColor: LingmoUI.Theme.highlightedTextColor
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                readOnly: !graphPointsSpinBox.editable
                                validator: graphPointsSpinBox.validator
                                selectByMouse: true
                            }

                            background: Rectangle {
                                implicitWidth: 120
                                implicitHeight: 36
                                color: LingmoUI.Theme.backgroundColor
                                radius: LingmoUI.Theme.smallRadius
                            }

                            down.indicator: Rectangle {
                                x: 0
                                width: 28
                                height: parent.height
                                color: graphPointsSpinBox.down.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       graphPointsSpinBox.down.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "−"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }

                            up.indicator: Rectangle {
                                x: parent.width - width
                                width: 28
                                height: parent.height
                                color: graphPointsSpinBox.up.pressed ? Qt.rgba(0, 0, 0, 0.15) :
                                       graphPointsSpinBox.up.hovered ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: height / 2

                                Label {
                                    anchors.centerIn: parent
                                    text: "+"
                                    color: LingmoUI.Theme.textColor
                                    opacity: enabled ? 1.0 : 0.3
                                }
                            }

                            onValueModified: {
                                settings.graphPoints = value
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                implicitHeight: processLayout.implicitHeight + 8 * LingmoUI.Units.largeSpacing
                title: qsTr("Process List")

                ColumnLayout {
                    id: processLayout
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing

                    RowLayout {
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("Default Sort By:")
                            Layout.preferredWidth: 120
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.7
                        }

                        ComboBox {
                            id: sortByBox
                            Layout.preferredWidth: 200
                            model: [qsTr("CPU Usage"), qsTr("Memory Usage"), qsTr("Process Name"), qsTr("User")]
                            currentIndex: 0
                            onCurrentIndexChanged: {
                                var sortFields = ["cpu", "memory", "name", "user"]
                                var field = sortFields[currentIndex]
                                processes.sortBy = field
                                settingsManager.defaultSortBy = field
                            }

                            background: Rectangle {
                                implicitHeight: 36
                                radius: LingmoUI.Theme.smallRadius
                                color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                             : Qt.rgba(0, 0, 0, 0.05)
                            }

                            delegate: ItemDelegate {
                                width: sortByBox.width
                                height: 36
                                highlighted: sortByBox.highlightedIndex === index

                                contentItem: Label {
                                    text: modelData
                                    color: highlighted ? LingmoUI.Theme.highlightedTextColor 
                                                     : LingmoUI.Theme.textColor
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: LingmoUI.Units.smallSpacing
                                }

                                background: Rectangle {
                                    radius: LingmoUI.Theme.smallRadius
                                    color: highlighted ? LingmoUI.Theme.highlightColor : "transparent"
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("Default Order:")
                            Layout.preferredWidth: 120
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.7
                        }

                        ComboBox {
                            id: sortOrderBox
                            Layout.preferredWidth: 200
                            model: [qsTr("Descending"), qsTr("Ascending")]
                            currentIndex: 0
                            onCurrentIndexChanged: {
                                var ascending = currentIndex === 1
                                processes.sortAscending = ascending
                                settingsManager.sortAscending = ascending
                            }

                            background: Rectangle {
                                implicitHeight: 36
                                radius: LingmoUI.Theme.smallRadius
                                color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                             : Qt.rgba(0, 0, 0, 0.05)
                            }

                            delegate: ItemDelegate {
                                width: sortOrderBox.width
                                height: 36
                                highlighted: sortOrderBox.highlightedIndex === index

                                contentItem: Label {
                                    text: modelData
                                    color: highlighted ? LingmoUI.Theme.highlightedTextColor 
                                                     : LingmoUI.Theme.textColor
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: LingmoUI.Units.smallSpacing
                                }

                                background: Rectangle {
                                    radius: LingmoUI.Theme.smallRadius
                                    color: highlighted ? LingmoUI.Theme.highlightColor : "transparent"
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
} 