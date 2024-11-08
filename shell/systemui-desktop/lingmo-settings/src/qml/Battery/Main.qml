import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    id: batteryPage
    headerTitle: qsTr("Battery")

    property int historyType: BatteryHistoryModel.ChargeType
    readonly property var timespanComboChoices: [qsTr("Last hour"),qsTr("Last 2 hours"),qsTr("Last 12 hours"),qsTr("Last 24 hours"),qsTr("Last 48 hours"), qsTr("Last 7 days")]
    readonly property var timespanComboDurations: [3600, 7200, 43200, 86400, 172800, 604800]

    readonly property int chargePercent: battery.chargePercent

    onChargePercentChanged: {
        valueAnimation.restart()
    }

    NumberAnimation {
        id: valueAnimation
        target: percentLabel
        property: "value"
        from: percentLabel.value
        to: batteryPage.chargePercent
        duration: 500
    }

    Battery {
        id: battery

        Component.onCompleted: {
            // battery.refresh()
            batteryBackground.value = battery.chargePercent
        }
    }

    BatteryHistoryModel {
        id: history
        duration: timespanComboDurations[3]
        device: battery.udi
        type: BatteryHistoryModel.ChargeType
    }

    Connections {
        target: battery

        function onChargePercentChanged(value) {
            batteryBackground.value = battery.chargePercent
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight
        visible: battery.available

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing * 2

            // Battery Info
            BatteryItem {
                id: batteryBackground
                Layout.fillWidth: true
                enableAnimation: !battery.onBattery
                height: 150

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: batteryBackground.radius + LingmoUI.Units.smallSpacing

                    Item {
                        Layout.fillHeight: true
                    }

                    RowLayout {
                        Label {
                            property int value: 0

                            id: percentLabel
                            text: value
                            color: "white"
                            font.pointSize: 40
                            font.weight: Font.DemiBold
                        }

                        Label {
                            text: "%"
                            color: "white"
                            font.pointSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }

                        Image {
                            id: sensorsVoltage
                            width: 30
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: "qrc:/images/sensors-voltage-symbolic.svg"
                            visible: !battery.onBattery
                        }
                    }

                    Label {
                        text: battery.statusString
                        color: "white"
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            RoundedItem {
                visible: history.count > 2
                spacing: 0

                Label {
                    text: qsTr("History")
                    color: LingmoUI.Theme.disabledTextColor
                }

                HistoryGraph {
                    Layout.fillWidth: true
                    height: 300

                    data: history.points

                    readonly property real xTicksAtDontCare: 0
                    readonly property real xTicksAtTwelveOClock: 1
                    readonly property real xTicksAtFullHour: 2
                    readonly property real xTicksAtHalfHour: 3
                    readonly property real xTicksAtFullSecondHour: 4
                    readonly property real xTicksAtTenMinutes: 5
                    readonly property var xTicksAtList: [xTicksAtTenMinutes, xTicksAtHalfHour, xTicksAtHalfHour,
                                                         xTicksAtFullHour, xTicksAtFullSecondHour, xTicksAtTwelveOClock]

                    // Set grid lines distances which directly correspondent to the xTicksAt variables
                    readonly property var xDivisionWidths: [1000 * 60 * 10, 1000 * 60 * 60 * 12, 1000 * 60 * 60, 1000 * 60 * 30, 1000 * 60 * 60 * 2, 1000 * 60 * 10]
                    xTicksAt: xTicksAtList[4]
                    xDivisionWidth: xDivisionWidths[xTicksAt]

                    xMin: history.firstDataPointTime
                    xMax: history.lastDataPointTime
                    xDuration: history.duration

                    yUnits: batteryPage.historyType === BatteryHistoryModel.RateType ? qsTr("W") : "%"
                    yMax: {
                        if (batteryPage.historyType === BatteryHistoryModel.RateType) {
                            // ceil to nearest 10
                            var max = Math.floor(history.largestValue)
                            max = max - max % 10 + 10

                            return max;
                        } else {
                            return 100;
                        }
                    }
                    yStep: batteryPage.historyType === BatteryHistoryModel.RateType ? 10 : 20
                    visible: history.count > 2
                }
            }

            RoundedItem {
                visible: battery.capacity

                Label {
                    text: qsTr("Health")
                    color: LingmoUI.Theme.disabledTextColor
                }

                RowLayout {
                    Layout.topMargin: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing * 4

                    // Poor
                    Item {
                        height: _poorLabel.implicitHeight + 4 + LingmoUI.Units.smallSpacing
                        width: _poorLabel.implicitWidth + LingmoUI.Units.largeSpacing

                        Rectangle {
                            id: _poorRect
                            anchors.fill: parent
                            color: "#FF8738"
                            radius: LingmoUI.Theme.mediumRadius
                            opacity: 0.1
                            visible: battery.capacity >= 0 && battery.capacity <= 79
                        }

                        Label {
                            id: _poorLabel
                            anchors.centerIn: parent
                            text: qsTr("Poor")
                            color: "#FF8738"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Item {
                        height: _normalLabel.implicitHeight + 4 + LingmoUI.Units.smallSpacing
                        width: _normalLabel.implicitWidth + LingmoUI.Units.largeSpacing

                        Rectangle {
                            id: _normalRect
                            anchors.fill: parent
                            color: "#3385FF"
                            radius: LingmoUI.Theme.mediumRadius
                            opacity: 0.1
                            visible: battery.capacity >= 80 && battery.capacity <= 89
                        }

                        Label {
                            id: _normalLabel
                            anchors.centerIn: parent
                            text: qsTr("Normal")
                            color: "#3385FF"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Item {
                        height: _excellentLabel.implicitHeight + 4 + LingmoUI.Units.smallSpacing
                        width: _excellentLabel.implicitWidth + LingmoUI.Units.largeSpacing

                        Rectangle {
                            id: _excellentRect
                            anchors.fill: parent
                            color: "#00CD23"
                            radius: LingmoUI.Theme.mediumRadius
                            opacity: 0.1
                            visible: battery.capacity >= 90 && battery.capacity <= 100
                        }

                        Label {
                            id: _excellentLabel
                            anchors.centerIn: parent
                            text: qsTr("Excellent")
                            color: "#00CD23"
                        }
                    }
                }

                HorizontalDivider {}

                StandardItem {
                    key: qsTr("Last Charged to") + " " + battery.lastChargedPercent + "%"
                    value: battery.lastChargedTime
                    visible: battery.lastChargedPercent !== 0
                }

                StandardItem {
                    key: qsTr("Maximum Capacity")
                    value: battery.capacity + "%"
                }
            }

            RoundedItem {
                RowLayout {
                    Label {
                        text: qsTr("Show percentage in status bar")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        checked: battery.showPercent
                        onCheckedChanged: battery.setPercentEnabled(checked)
                        rightPadding: 0
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing
            }
        }
    }

    Label {
        anchors.centerIn: parent
        text: qsTr("No battery found")
        visible: !battery.available
    }
}
