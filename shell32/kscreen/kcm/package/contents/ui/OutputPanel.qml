/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.kcm 1.6 as KCM
import org.kde.private.kcm.kscreen 1.0 as KScreen

Kirigami.FormLayout {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property var element: model

    signal reorder()

    QQC2.CheckBox {
       text: i18n("Enabled")
       checked: element.enabled
       onToggled: element.enabled = checked
       visible: kcm.outputModel.rowCount() > 1
    }

    RowLayout {
        visible: kcm.primaryOutputSupported && root.enabledOutputs.count >= 2

        QQC2.Button {
            visible: root.enabledOutputs.count >= 3
            text: i18n("Change Screen Priorities…")
            icon.name: "document-edit"
            onClicked: root.reorder();
        }

        QQC2.RadioButton {
            visible: root.enabledOutputs.count === 2
            text: i18n("Primary")
            checked: element.priority === 1
            onToggled: element.priority = 1
        }

        KCM.ContextualHelpButton {
            toolTipText: xi18nc("@info", "This determines which screen your main desktop and panel appear on. Some older games also use it to decide which screen to appear on.<nl/><nl/>It has no effect on what screen notifications or other windows appear on.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Resolution:")

        QQC2.ComboBox {
            id: resolutionCombobox
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            visible: count > 1
            model: element.resolutions
            onActivated: element.resolutionIndex = currentIndex;
            Component.onCompleted: currentIndex = element.resolutionIndex;
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleResolutionLabel
            visible: resolutionCombobox.count <= 1
            text: element.resolutions[0] || ""
        }
        KCM.ContextualHelpButton {
            visible: resolutionCombobox.count <= 1
            toolTipText: xi18nc("@info", "&quot;%1&quot; is the only resolution supported by this display.<nl/><nl/>Using unsupported resolutions was possible in the Plasma X11 session, but they were never guaranteed to work and are not available in this Plasma Wayland session.", singleResolutionLabel.text)
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16

        visible: kcm.perOutputScaling
        Kirigami.FormData.label: i18n("Scale:")

        QQC2.Slider {
            id: scaleSlider

            Layout.fillWidth: true
            from: 0.5
            to: 3
            stepSize: 0.25
            live: true
            value: element.scale
            onMoved: element.scale = value
        }
        QQC2.SpinBox {
            id: spinbox
            // Because QQC2 SpinBox doesn't natively support decimal step
            // sizes: https://bugreports.qt.io/browse/QTBUG-67349
            readonly property real factor: 20.0
            readonly property real realValue: value / factor

            from : 0.5 * factor
            to : 3.0 * factor
            stepSize: 1
            value: element.scale * factor
            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
            }
            textFromValue: (value, locale) =>
                i18nc("Global scale factor expressed in percentage form", "%1%",
                    parseFloat(value * 1.0 / factor * 100.0))
            valueFromText: (text, locale) =>
                Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0

            onValueModified: element.scale = realValue
        }
    }

    Orientation {}

    RowLayout {
        Kirigami.FormData.label: i18n("Refresh rate:")

        QQC2.ComboBox {
            id: refreshRateCombobox
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            visible: count > 1
            model: element.refreshRates
            onActivated: element.refreshRateIndex = currentIndex;
            Component.onCompleted: currentIndex = element.refreshRateIndex;
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleRefreshRateLabel
            visible: refreshRateCombobox.count <= 1
            text: element.refreshRates[0] || ""
        }
        KCM.ContextualHelpButton {
            visible: refreshRateCombobox.count <= 1
            toolTipText: i18n("\"%1\" is the only refresh rate supported by this display.", singleRefreshRateLabel.text)
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Adaptive sync:")
        Layout.minimumWidth: Kirigami.Units.gridUnit * 11
        model: [
            { label: i18n("Never"), value: KScreen.Output.VrrPolicy.Never },
            { label: i18n("Always"), value: KScreen.Output.VrrPolicy.Always },
            { label: i18n("Automatic"), value: KScreen.Output.VrrPolicy.Automatic }
        ]
        textRole: "label"
        // valueRole: "value"
        visible: element.capabilities & KScreen.Output.Capability.Vrr

        onActivated: element.vrrPolicy = currentValue;
        Component.onCompleted: currentIndex = indexOfValue(element.vrrPolicy);
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Overscan:")
        visible: element.capabilities & KScreen.Output.Capability.Overscan

        QQC2.SpinBox {
            from: 0
            to: 100
            value: element.overscan
            onValueModified: element.overscan = value
            textFromValue: function(value, locale) {
                return value + '%';
            }
            valueFromText: function(text, locale) {
                return parseInt(text.replace("%", ""))
            }
        }

        KCM.ContextualHelpButton {
            toolTipText: xi18nc("@info", `Determines how much padding is put around the image sent to the display
                                          to compensate for part of the content being cut off around the edges.<nl/><nl/>
                                          This is sometimes needed when using a TV as a screen`)
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("RGB Range:")
        visible: element.capabilities & KScreen.Output.Capability.RgbRange

        QQC2.ComboBox {
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("Automatic"), value: KScreen.Output.RgbRange.Automatic },
                { label: i18n("Full"), value: KScreen.Output.RgbRange.Full },
                { label: i18n("Limited"), value: KScreen.Output.RgbRange.Limited }
            ]
            textRole: "label"
            // valueRole: "value"

            onActivated: element.rgbRange = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.rgbRange);
        }

        KCM.ContextualHelpButton {
            toolTipText: xi18nc("@info", `Determines whether or not the range of possible color values needs to be limited for the display.
                                          This should only be changed if the colors on the screen look washed out.`)
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Replica of:")
        Layout.minimumWidth: Kirigami.Units.gridUnit * 11
        model: element.replicationSourceModel
        visible: kcm.outputReplicationSupported && kcm.outputModel && kcm.outputModel.rowCount() > 1

        onModelChanged: enabled = (count > 1);
        onCountChanged: enabled = (count > 1);

        Component.onCompleted: currentIndex = element.replicationSourceIndex;
        onActivated: element.replicationSourceIndex = currentIndex;
    }
}
