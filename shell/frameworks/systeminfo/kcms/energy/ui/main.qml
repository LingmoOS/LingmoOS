/*
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.kquickcontrolsaddons 2.0
import org.kde.kinfocenter.energy.private 1.0

import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property QtObject currentBattery: null
    property string currentUdi: ""
    property string currentVendor: ""
    property string currentProduct: ""
    property bool compact: (root.width / LingmoUI.Units.gridUnit) < 25

    function initCurrentBattery() {
        currentBattery = kcm.batteries.data(kcm.batteries.index(0, 0), BatteryModel.BatteryRole)
        currentVendor = kcm.batteries.data(kcm.batteries.index(0, 0), BatteryModel.VendorRole)
        currentProduct = kcm.batteries.data(kcm.batteries.index(0, 0), BatteryModel.ProductRole)
        currentUdi = kcm.batteries.data(kcm.batteries.index(0, 0), BatteryModel.UdiRole)
    }

    Component.onCompleted: initCurrentBattery()

    onCurrentBatteryChanged: {
        if (!currentBattery) {
            initCurrentBattery()
        }
    }

    property int historyType: HistoryModel.ChargeType

    readonly property var details: [
        {
            title: i18n("Battery"),
            data: [
                {label: i18n("Rechargeable"), value: "rechargeable"},
                {label: i18n("Charge state"), value: "chargeState", modifier: "chargeState"},
                {label: i18n("Current charge"), value: "chargePercent", unit: "%", precision: 0},
                {label: i18n("Health"), value: "capacity", unit: "%", precision: 0},
                {label: i18n("Vendor"), value: "vendor", source:"Vendor"},
                {label: i18n("Model"), value: "model", source:"Product"},
                {label: i18n("Serial Number"), value: "serial"},
                {label: i18n("Technology"), value: "technology", modifier: "technology"}
            ]
        },
        {
            title: i18n("Energy"),
            data: [
                {label: i18nc("current power draw from the battery in W", "Consumption"), value: "energyRate", unit: i18nc("Watt", "W"), precision: 2},
                {label: i18n("Voltage"), value: "voltage", unit: i18nc("Volt", "V"), precision: 2},
                {label: i18n("Remaining energy"), value: "energy", unit: i18nc("Watt-hours", "Wh"), precision: 2},
                {label: i18n("Last full charge"), value: "energyFull", unit: i18nc("Watt-hours", "Wh"), precision: 2},
                {label: i18n("Original charge capacity"), value: "energyFullDesign", unit: i18nc("Watt-hours", "Wh"), precision: 2}
            ]
        },
        {
            title: i18n("Environment"),
            data: [
                {label: i18n("Temperature"), value: "temperature", unit: i18nc("Degree Celsius", "°C"), precision: 2}
            ]
        },
    ]

    function modifier_chargeState(value) {
        switch(value) {
        case Battery.NoCharge: return i18n("Not charging")
        case Battery.Charging: return i18n("Charging")
        case Battery.Discharging: return i18n("Discharging")
        case Battery.FullyCharged: return i18n("Fully charged")
        }
    }

    function modifier_technology(value) {
        switch(value) {
        case Battery.LithiumIon: return i18n("Lithium ion")
        case Battery.LithiumPolymer: return i18n("Lithium polymer")
        case Battery.LithiumIronPhosphate: return i18n("Lithium iron phosphate")
        case Battery.LeadAcid: return i18n("Lead acid")
        case Battery.NickelCadmium: return i18n("Nickel cadmium")
        case Battery.NickelMetalHydride: return i18n("Nickel metal hydride")
        default: return i18n("Unknown technology")
        }
    }

    implicitWidth: LingmoUI.Units.gridUnit * 30
    implicitHeight: !!currentBattery ? LingmoUI.Units.gridUnit * 30 : LingmoUI.Units.gridUnit * 12

    readonly property var timespanComboChoices: [i18n("Last hour"),i18n("Last 2 hours"),i18n("Last 12 hours"),i18n("Last 24 hours"),i18n("Last 48 hours"), i18n("Last 7 days")]
    readonly property var timespanComboDurations: [3600, 7200, 43200, 86400, 172800, 604800]

    LingmoUI.PlaceholderMessage {
        anchors.centerIn: parent
        visible: kcm.batteries.count <= 0 && history.count <= 0
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
        text: i18nc("@info:status", "No energy information available on this system")
        icon.name: "utilities-energy-monitor"
    }

    ColumnLayout {
        id: column
        QQC2.ScrollView {
            id: tabView
            Layout.fillWidth: true
            Layout.minimumHeight: LingmoUI.Units.gridUnit * 4
            Layout.maximumHeight: Layout.minimumHeight
            visible: kcm.batteries.count > 1

            Row {
                spacing: LingmoUI.Units.smallSpacing
                Repeater {
                    model: kcm.batteries

                    QQC2.Button {
                        id: button
                        width: LingmoUI.Units.gridUnit * 12
                        height: tabView.height
                        checked: model.battery == root.currentBattery
                        checkable: true
                        onClicked: {
                            root.currentUdi = model.udi
                            root.currentVendor = model.vendor
                            root.currentProduct = model.product
                            root.currentBattery = model.battery

                            // override checked property
                            checked = Qt.binding(function() {
                                return model.battery == root.currentBattery
                            })
                        }

                        ColumnLayout {
                            anchors {
                                fill: parent
                                margins: LingmoUI.Units.smallSpacing * 2
                            }
                            RowLayout {
                                // Reference: BatteryType enum in solid/devices/frontend/battery.h
                                LingmoUI.Icon {
                                    id: batteryIcon
                                    Layout.preferredWidth: LingmoUI.Units.iconSizes.smallMedium
                                    Layout.preferredHeight: Layout.preferredWidth
                                    source: {
                                        switch(model.battery.type) {
                                            case 1: return "phone"
                                            case 2: return "battery-ups"
                                            case 3: return model.battery.chargeState === 1 ? "battery-full-charging" : "battery-full"
                                            case 4: return "input-mouse"
                                            case 5: return "input-keyboard"
                                            case 6: return "input-keyboard" // TODO: New Icon Required?
                                            case 7: return "camera-photo"
                                            case 8: return "smartphone"
                                            case 9: return "monitor"
                                            case 10: return "input-gamepad"
                                            case 11: return "preferences-system-bluetooth-battery"
                                            case 12: return "input-tablet"
                                            case 13: return "headphone"
                                            case 14: return "headset"
                                            case 15: return "input-touchpad"
                                            default: return "paint-unknown"
                                        }
                                    }
                                }

                                ColumnLayout {
                                    spacing: 0

                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        text: {
                                            switch(model.battery.type) {
                                                case 1: return i18n("PDA Battery")
                                                case 2: return i18n("UPS Battery")
                                                case 3: return i18n("Internal Battery")
                                                case 4: return i18n("Mouse Battery")
                                                case 5: return i18n("Keyboard Battery")
                                                case 6: return i18n("Keyboard/Mouse Battery")
                                                case 7: return i18n("Camera Battery")
                                                case 8: return i18n("Phone Battery")
                                                case 9: return i18n("Monitor Battery")
                                                case 10: return i18n("Gaming Input Battery")
                                                case 11: return i18n("Bluetooth Battery")
                                                case 12: return i18n("Tablet Battery")
                                                case 13: return i18n("Headphone Battery")
                                                case 14: return i18n("Headset Battery")
                                                case 15: return i18n("Touchpad Battery")
                                                default: return i18n("Unknown Battery")
                                            }
                                            elide: Text.ElideRight
                                            maximumLineCount : 1
                                        }
                                    }

                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        text: model.product
                                        color: LingmoUI.Theme.disabledTextColor
                                        elide: Text.ElideRight
                                        maximumLineCount : 1
                                    }
                               }
                            }

                            RowLayout {
                                QQC2.ProgressBar {
                                    id: percentageSlider
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 100
                                    value: model.battery.chargePercent
                                }

                                QQC2.Label {
                                    text: model.battery.chargeState === 1 ?
                                        i18nc("Battery charge percentage", "%1% (Charging)", Math.round(percentageSlider.value)) :
                                        i18nc("Battery charge percentage", "%1%", Math.round(percentageSlider.value))
                                }
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: LingmoUI.Units.smallSpacing
            visible: !!currentBattery


            HistoryModel {
                id: history
                duration: timespanComboDurations[timespanCombo.currentIndex]
                device: currentUdi
                type: root.historyType
            }

            Graph {
                id: graph
                Layout.fillWidth: true
                Layout.minimumHeight: column.width / 3
                Layout.maximumHeight: column.width / 3
                Layout.topMargin: -LingmoUI.Units.largeSpacing

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
                xTicksAt: xTicksAtList[timespanCombo.currentIndex]
                xDivisionWidth: xDivisionWidths[xTicksAt]

                xMin: history.firstDataPointTime
                xMax: history.lastDataPointTime
                xDuration: history.duration

                yUnits: root.historyType == HistoryModel.RateType ? i18nc("Shorthand for Watts","W") : i18nc("literal percent sign","%")
                yMax: {
                    if (root.historyType == HistoryModel.RateType) {
                        // ceil to nearest 10
                        var max = Math.floor(history.largestValue)
                        max = max - max % 10 + 10

                        return max;
                    } else {
                        return 100;
                    }
                }
                yStep: root.historyType == HistoryModel.RateType ? 10 : 20
                visible: history.count > 1
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: graph.xPadding
                Layout.rightMargin: graph.xPadding/2
                columns: !compact ? 5 : 3

                QQC2.Button {
                    id: chargeButton
                    checked: true
                    checkable: true
                    text: i18n("Charge Percentage")
                    onClicked: {
                        historyType = HistoryModel.ChargeType
                        rateButton.checked = false
                    }
                }

                QQC2.Button {
                    id: rateButton
                    checkable: true
                    text: i18n("Energy Consumption")
                    onClicked: {
                        historyType = HistoryModel.RateType
                        chargeButton.checked = false
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                QQC2.ComboBox {
                    id: timespanCombo
                    Layout.minimumWidth: LingmoUI.Units.gridUnit * 6
                    model: timespanComboChoices
                    Accessible.name: i18n("Timespan")
                    Accessible.description: i18n("Timespan of data to display")
                }

                QQC2.Button {
                    icon.name: "view-refresh-symbolic"
                    hoverEnabled: true
                    QQC2.ToolTip.text: i18n("Refresh")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                    Accessible.name: QQC2.ToolTip.text
                    onClicked: history.refresh()
                }
            }

            LingmoUI.InlineMessage {
                Layout.fillWidth: true
                Layout.topMargin: LingmoUI.Units.smallSpacing
                showCloseButton: true
                text: i18n("This type of history is currently not available for this device.")
                visible: !graph.visible
            }
        }

        ColumnLayout {
            id: detailsColumn
            spacing: 0

            Layout.fillWidth: true
            visible: !!currentBattery

            Repeater {
                id: titleRepeater
                model: root.details
                property list<LingmoUI.FormLayout> layouts

                delegate: LingmoUI.FormLayout {
                    id: currentLayout

                    Component.onCompleted: {
                        // ensure that all visible FormLayout share the same set of twinFormLayouts
                        titleRepeater.layouts.push(currentLayout);
                        for (var i = 0, length = titleRepeater.layouts.length; i < length; ++i) {
                            titleRepeater.layouts[i].twinFormLayouts = titleRepeater.layouts;
                        }
                    }

                    LingmoUI.Heading {
                        text: modelData.title
                        LingmoUI.FormData.isSection: true
                        level: 2
                        // HACK hide section header if all labels are invisible
                        visible: {
                            for (var i = 0, length = detailsRepeater.count; i < length; ++i) {
                                var item = detailsRepeater.itemAt(i)
                                if (item && item.visible) {
                                    return true
                                }
                            }

                            return false
                        }
                    }

                    Repeater {
                        id: detailsRepeater
                        model: modelData.data || []

                        LingmoUI.SelectableLabel {
                            id: valueLabel
                            Layout.fillWidth: true
                            Keys.onPressed: {
                                if (event.matches(StandardKey.Copy)) {
                                    valueLabel.copy();
                                    event.accepted = true;
                                }
                            }
                            LingmoUI.FormData.label: i18n("%1:", modelData.label)
                            text: {
                                var value;
                                if (modelData.source) {
                                    value = root["current" + modelData.source];
                                } else {
                                    value = currentBattery[modelData.value]
                                }

                                if (typeof value === "boolean") {
                                    if (value) {
                                        return i18n("Yes")
                                    } else {
                                        return i18n("No")
                                    }
                                }

                                if (!value) {
                                    return ""
                                }

                                var precision = modelData.precision
                                if (typeof precision === "number") { // round to decimals
                                    value = Number(value).toLocaleString(Qt.locale(), "f", precision)
                                }

                                if (modelData.modifier && root["modifier_" + modelData.modifier]) {
                                    value = root["modifier_" + modelData.modifier](value)
                                }

                                if (modelData.unit) {
                                    if (modelData.unit === "%") {
                                        // We delay the percentage localization as the position may vary
                                        value = i18nc("%1 is a percentage value", "%1%", value)
                                    } else {
                                        value = i18nc("%1 is value, %2 is unit", "%1 %2", value, modelData.unit)
                                    }
                                }

                                return value
                            }
                            visible: valueLabel.text !== ""
                        }
                    }
                }
            }
        }
    }
}
