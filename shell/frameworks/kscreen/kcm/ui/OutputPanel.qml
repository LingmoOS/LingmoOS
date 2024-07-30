/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.private.kcm.kscreen 1.0 as KScreen

Kirigami.FormLayout {
    id: root

    property KSortFilterProxyModel enabledOutputs
    property var element: model

    readonly property bool hdrAvailable: (element.capabilities & KScreen.Output.Capability.HighDynamicRange) && (element.capabilities & KScreen.Output.Capability.WideColorGamut)

    signal reorder()

    QQC2.CheckBox {
       text: i18n("Enabled")
       checked: element.enabled
       onToggled: element.enabled = checked
       visible: kcm.multipleScreensAvailable
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

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "This determines which screen your main desktop appears on, along with any Plasma Panels in it. Some older games also use this setting to decide which screen to appear on.<nl/><nl/>It has no effect on what screen notifications or other windows appear on.")
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
            Component.onCompleted: currentIndex = Qt.binding(() => element.resolutionIndex);
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleResolutionLabel
            visible: resolutionCombobox.count <= 1
            text: element.resolutions[0] || ""
        }
        Kirigami.ContextualHelpButton {
            visible: resolutionCombobox.count <= 1
            toolTipText: xi18nc("@info", "&quot;%1&quot; is the only resolution supported by this display.", singleResolutionLabel.text)
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

            from: 0.5 * factor
            to: 3.0 * factor
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
            Component.onCompleted: currentIndex = Qt.binding(() => element.refreshRateIndex);
        }
        // When the combobox is has only one item, it's basically non-interactive
        // and is serving purely in a descriptive role, so make this explicit by
        // using a label instead
        QQC2.Label {
            id: singleRefreshRateLabel
            visible: refreshRateCombobox.count <= 1
            text: element.refreshRates[0] || ""
        }
        Kirigami.ContextualHelpButton {
            visible: refreshRateCombobox.count <= 1
            toolTipText: i18n("\"%1\" is the only refresh rate supported by this display.", singleRefreshRateLabel.text)
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Adaptive sync:")
        Layout.minimumWidth: Kirigami.Units.gridUnit * 11
        model: [
            { label: i18n("Never"), value: KScreen.Output.VrrPolicy.Never },
            { label: i18n("Automatic"), value: KScreen.Output.VrrPolicy.Automatic },
            { label: i18n("Always"), value: KScreen.Output.VrrPolicy.Always },
        ]
        textRole: "label"
        valueRole: "value"
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
            textFromValue: (value, locale) =>
            i18nc("Overscan expressed in percentage form", "%1%", value)
            valueFromText: (text, locale) =>
            Number.fromLocaleString(locale, text.replace("%", ""))
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "Determines how much padding is put around the image sent to the display to compensate for part of the content being cut off around the edges.<nl/><nl/>This is sometimes needed when using a TV as a screen")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("RGB range:")
        visible: element.capabilities & KScreen.Output.Capability.RgbRange

        QQC2.ComboBox {
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("Automatic"), value: KScreen.Output.RgbRange.Automatic },
                { label: i18n("Full"), value: KScreen.Output.RgbRange.Full },
                { label: i18n("Limited"), value: KScreen.Output.RgbRange.Limited }
            ]
            textRole: "label"
            valueRole: "value"

            onActivated: element.rgbRange = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.rgbRange);
        }

        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "Determines whether or not the range of possible color values needs to be limited for the display. This should only be changed if the colors on the screen look washed out.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label:combobox", "Color Profile:")
        visible: element.capabilities & KScreen.Output.Capability.IccProfile
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            enabled: !element.hdr || !root.hdrAvailable
            Layout.minimumWidth: Kirigami.Units.gridUnit * 11
            model: [
                { label: i18n("None"), value: KScreen.Output.ColorProfileSource.sRGB },
                { label: i18n("ICC Profile"), value: KScreen.Output.ColorProfileSource.ICC },
                { label: i18n("Built-In"), value: KScreen.Output.ColorProfileSource.EDID }
            ]
            textRole: "label"
            valueRole: "value"

            onActivated: element.colorProfileSource = currentValue;
            Component.onCompleted: currentIndex = indexOfValue(element.colorProfileSource);
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Note that built-in color profiles are sometimes wrong, and often inaccurate. For optimal color fidelity, calibration using a colorimeter is recommended.")
            visible: (!element.hdr || !root.hdrAvailable) && element.colorProfileSource == KScreen.Output.ColorProfileSource.EDID
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "The built-in color profile is always used with HDR.")
            visible: element.hdr && root.hdrAvailable
        }
    }

    RowLayout {
        visible: (element.capabilities & KScreen.Output.Capability.IccProfile) && (element.colorProfileSource == KScreen.Output.ColorProfileSource.ICC)
        spacing: Kirigami.Units.smallSpacing

        Kirigami.ActionTextField {
            id: iccProfileField
            onTextChanged: element.iccProfilePath = text
            onTextEdited: element.iccProfilePath = text
            placeholderText: i18nc("@info:placeholder", "Enter ICC profile path…")
            enabled: !root.hdrAvailable || !element.hdr

            rightActions: Kirigami.Action {
                icon.name: "edit-clear-symbolic"
                visible: iccProfileField.text !== ""
                onTriggered: {
                    iccProfileField.text = ""
                }
            }

            Component.onCompleted: text = element.iccProfilePath;
        }

        QQC2.Button {
            icon.name: "document-open-symbolic"
            text: i18nc("@action:button", "Select ICC profile…")
            display: QQC2.AbstractButton.IconOnly
            onClicked: fileDialogComponent.incubateObject(root);
            enabled: !root.hdrAvailable || !element.hdr

            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            Accessible.role: Accessible.Button
            Accessible.name: text
            Accessible.description: i18n("Opens a file picker for the ICC profile")
            Accessible.onPressAction: onClicked();
        }

        Component {
            id: fileDialogComponent

            FileDialog {
                id: fileDialog
                title: i18nc("@title:window", "Select ICC Profile")
                currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
                nameFilters: ["ICC profiles (*.icc *.icm)"]

                onAccepted: {
                    iccProfileField.text = urlToProfilePath(selectedFile);
                    destroy();
                }
                onRejected: destroy();
                Component.onCompleted: open();

                function urlToProfilePath(qmlUrl) {
                    const url = new URL(qmlUrl);
                    let path = decodeURIComponent(url.pathname);
                    // Remove the leading slash from the url
                    if (url.protocol === "file:" && path.charAt(1) === ':') {
                        path = path.substring(1);
                    }
                    return path;
                }
            }
        }

        Kirigami.ContextualHelpButton {
            visible: root.hdrAvailable && element.hdr
            toolTipText: i18nc("@info:tooltip", "ICC profiles aren't compatible with HDR yet")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label", "High Dynamic Range:")
        visible: root.hdrAvailable
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            text: i18nc("@option:check", "Enable HDR")
            checked: element.hdr
            onToggled: element.hdr = checked
        }

        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "HDR allows compatible applications to show brighter and more vivid colors. Note that this feature is still experimental")
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        spacing: Kirigami.Units.smallSpacing

        visible: root.hdrAvailable && element.hdr
        Kirigami.FormData.label: i18nc("@label", "SDR Brightness:")

        QQC2.Slider {
            Layout.fillWidth: true
            from: 50
            to: element.peakBrightness === 0 ? 500 : element.peakBrightness
            stepSize: 50
            live: true
            value: element.sdrBrightness
            onMoved: element.sdrBrightness = value
        }
        QQC2.SpinBox {
            from: 50
            to: element.peakBrightness === 0 ? 500 : element.peakBrightness
            stepSize: 10
            value: element.sdrBrightness
            onValueModified: element.sdrBrightness = value
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Sets the brightness of non-HDR content on the screen, in nits")
        }
    }

    RowLayout {
        Layout.fillWidth: true
        // Set the same limit as the device ComboBox
        Layout.maximumWidth: Kirigami.Units.gridUnit * 16
        spacing: Kirigami.Units.smallSpacing

        visible: root.hdrAvailable && element.hdr
        Kirigami.FormData.label: i18nc("@label", "SDR Color Intensity:")

        QQC2.Slider {
            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 10
            live: true
            value: element.sdrGamutWideness * 100
            onMoved: element.sdrGamutWideness = value / 100.0
        }
        QQC2.SpinBox {
            // Because QQC2 SpinBox doesn't natively support decimal step
            // sizes: https://bugreports.qt.io/browse/QTBUG-67349
            readonly property real factor: 20.0
            readonly property real realValue: value / factor

            from: 0
            to: 1.0 * factor
            stepSize: 1
            value: element.sdrGamutWideness * factor
            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to) * spinbox.factor
                top:  Math.max(spinbox.from, spinbox.to) * spinbox.factor
            }
            textFromValue: (value, locale) =>
            i18nc("Color intensity factor expressed in percentage form", "%1%",
                  parseFloat(value * 1.0 / factor * 100.0))
            valueFromText: (text, locale) =>
            Number.fromLocaleString(locale, text.replace("%", "")) * factor / 100.0

            onValueModified: element.sdrGamutWideness = realValue
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Increases the intensity of non-HDR content on the screen")
        }
    }

    QQC2.ComboBox {
        Kirigami.FormData.label: i18n("Replica of:")
        Layout.minimumWidth: Kirigami.Units.gridUnit * 11
        model: element.replicationSourceModel
        visible: kcm.outputReplicationSupported && kcm.multipleScreensAvailable

        onModelChanged: enabled = (count > 1);
        onCountChanged: enabled = (count > 1);

        Component.onCompleted: currentIndex = element.replicationSourceIndex;
        onActivated: element.replicationSourceIndex = currentIndex;
    }
}
