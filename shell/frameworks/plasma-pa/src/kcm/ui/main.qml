/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.coreaddons as KCoreAddons
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.plasma.private.volume

KCM.ScrollViewKCM {
    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    GlobalConfig {
        id: config
    }

    SinkModel {
        id: paSinkModel
    }

    SourceModel {
        id: paSourceModel
    }

    CardModel {
        id: paCardModel

        function indexOfCardNumber(cardNumber) {
            for (var idx = 0; idx < count; idx++) {
                if (data(index(idx, 0), KItemModels.KRoleNames.role("Index")) == cardNumber) {
                    return index(idx, 0);
                }
            }
            return index(-1, 0);
        }
    }

    PulseObjectFilterModel {
        id: paSinkFilterModel
        filterOutInactiveDevices: true
        sourceModel: paSinkModel
    }

    PulseObjectFilterModel {
        id: paSourceFilterModel
        filterOutInactiveDevices: true
        sourceModel: paSourceModel
    }

    ModuleManager {
        id: moduleManager
    }

    actions: [
        Kirigami.Action {
            id: inactiveDevicesSwitch
            icon.name: "view-visible"
            visible: (paSourceModel.count != paSourceFilterModel.count) || (paSinkModel.count != paSinkFilterModel.count)

            displayComponent: Switch {
                text: i18nd("kcm_pulseaudio", "Show Inactive Devices")

                onToggled: inactiveDevicesSwitch.checked = checked
            }
        },
        Kirigami.Action {
            icon.name: "configure"
            text: i18nd("kcm_pulseaudio", "Configure Volume Controls…")
            onTriggered: kcm.push("VolumeControlsConfig.qml", { "config": config })
        },
        Kirigami.Action {
            id: configureButton
            visible: moduleManager.settingsSupported
            enabled: moduleManager.configModuleLoaded
            text: i18nd("kcm_pulseaudio", "Configure…")
            icon.name: "configure"

            tooltip: i18nd("kcm_pulseaudio", "Requires %1 PulseAudio module", moduleManager.configModuleName)

            Kirigami.Action {
                text: i18nd("kcm_pulseaudio", "Add virtual output device for simultaneous output on all local sound cards")
                checkable: true
                checked: moduleManager.combineSinks
                onToggled: moduleManager.combineSinks = checked
            }
            Kirigami.Action {
                text: i18nd("kcm_pulseaudio", "Automatically switch all running streams when a new output becomes available")
                checkable: true
                checked: moduleManager.switchOnConnect
                onToggled: moduleManager.switchOnConnect = checked
            }
        }
    ]

    view: Flickable {
        id: flickable

        contentWidth: column.width
        contentHeight: column.height
        clip: true

        ColumnLayout {
            id: column
            width: flickable.width

            // Only show labels if both port/profile comboboxes are visible
            readonly property var comboBoxLabelsVisible: {
                for (var i = 0; i < sinks.count; ++i) {
                    let delegate = sinks.itemAtIndex(i)
                    if (delegate != null && delegate.portVisible)
                        return true
                }
                for (var i = 0; i < sources.count; ++i) {
                    let delegate = sources.itemAtIndex(i)
                    if (delegate != null && delegate.portVisible)
                        return true
                }
                return false
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: sinks.visible
                text: i18nd("kcm_pulseaudio", "Playback Devices")
            }

            ListView {
                id: sinks
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: inactiveDevicesSwitch.checked || !inactiveDevicesSwitch.visible ? paSinkModel : paSinkFilterModel
                delegate: DeviceListItem {
                    isPlayback: true
                    comboBoxLabelsVisible: column.comboBoxLabelsVisible
                }
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: sources.visible
                text: i18nd("kcm_pulseaudio", "Recording Devices")
            }

            ListView {
                id: sources
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: inactiveDevicesSwitch.checked || !inactiveDevicesSwitch.visible ? paSourceModel : paSourceFilterModel
                delegate: DeviceListItem {
                    isPlayback: false
                    comboBoxLabelsVisible: column.comboBoxLabelsVisible
                }
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: inactiveCards.visible
                text: i18nd("kcm_pulseaudio", "Inactive Cards")
            }

            ListView {
                id: inactiveCards
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: KItemModels.KSortFilterProxyModel {
                    sourceModel: paCardModel
                    filterRowCallback: function(source_row, source_parent) {
                        let idx = sourceModel.index(source_row, 0);
                        let profiles = sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role("Profiles"))
                        let activeProfileIndex = sourceModel.data(idx, sourceModel.KItemModels.KRoleNames.role("ActiveProfileIndex"))
                        return profiles[activeProfileIndex].name == "off";
                    }
                }
                delegate: CardListItem {
                    comboBoxLabelsVisible: column.comboBoxLabelsVisible
                }
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: eventStreamView.visible || sinkInputView.visible
                text: i18nd("kcm_pulseaudio", "Playback Streams")
            }

            ListView {
                id: eventStreamView
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                // Only have a bottom margin if the ListView beneath is empty and not visible
                Layout.bottomMargin: sinkInputView.visible ? 0 : Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: PulseObjectFilterModel {
                    filters: [ { role: "Name", value: "sink-input-by-media-role:event" } ]
                    sourceModel: StreamRestoreModel {}
                }
                delegate: StreamListItem {
                    deviceModel: paSinkModel
                    isPlayback: true
                }
            }

            ListView {
                id: sinkInputView
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: PulseObjectFilterModel {
                    filters: [ { role: "VirtualStream", value: false } ]
                    sourceModel: SinkInputModel {}
                }
                delegate: StreamListItem {
                    deviceModel: paSinkModel
                    isPlayback: true
                }
            }

            Kirigami.ListSectionHeader {
                Layout.fillWidth: true
                visible: sourceOutputView.visible
                text: i18nd("kcm_pulseaudio", "Recording Streams")
            }

            ListView {
                id: sourceOutputView
                visible: count > 0
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.preferredHeight: contentHeight
                interactive: false
                spacing: Kirigami.Units.largeSpacing
                model: PulseObjectFilterModel {
                    filters: [ { role: "VirtualStream", value: false } ]
                    sourceModel: SourceOutputModel {}
                }

                delegate: StreamListItem {
                    deviceModel: sourceModel
                    isPlayback: false
                }
            }
        }
    }

    SpeakerTest {
        id: tester
        sink: testOverlay.sinkObject

        onShowErrorMessage: message => {
            testError.text = i18ndc("kcm_pulseaudio",
                                    "%1 is an error string produced by an external component, and probably untranslated",
                                    "Error trying to play a test sound. \nThe system said: \"%1\"", message);
            testError.visible = true;
        }
    }

    Kirigami.OverlaySheet {
        id: testOverlay

        property var sinkObject: null
        property string description: ""
        property string iconName: "audio-card"
        property string profile: ""
        property string port: ""

        function testSink(index) {
            let modelIndex = sinks.model.index(Math.max(index, 0), 0);
            sinkObject = sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("PulseObject"));
            description = sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("Description"));
            iconName = sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("IconName")) || "audio-card";

            let ports = sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("Ports"));
            port = ports.length > 1 ? ports[sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("ActivePortIndex"))].description : "";

            profile = "";
            let cardIndex = paCardModel.indexOfCardNumber(sinks.model.data(modelIndex, sinks.model.KItemModels.KRoleNames.role("CardIndex")));
            if (cardIndex.valid) {
                let profiles = paCardModel.data(cardIndex, paCardModel.KItemModels.KRoleNames.role("Profiles")) || [];
                profile = profiles.length > 1 ? profiles[paCardModel.data(cardIndex, paCardModel.KItemModels.KRoleNames.role("ActiveProfileIndex"))].description : "";
            }

            testOverlay.open();
        }

        onSinkObjectChanged: {
            if (!sinkObject) {
                testOverlay.close();
            }
        }

        header: GridLayout {
            columns: 2
            rowSpacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: testOverlay.iconName || "audio-card"
                Layout.rowSpan: 3
                Layout.alignment: Qt.AlignCenter
            }
            Label {
                text: testOverlay.description
                font.bold: true
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Label {
                text: {
                    if (testOverlay.port.length === 0) { return testOverlay.profile }
                    if (testOverlay.profile.length === 0) { return testOverlay.port }
                    return testOverlay.profile + " / " + testOverlay.port
                }
                visible: text.length > 0
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        ColumnLayout {
            Kirigami.InlineMessage {
                id: testError
                type: Kirigami.MessageType.Error
                showCloseButton: true
                Layout.fillWidth: true
            }

            GridLayout {
                columns: 3
                rowSpacing: Kirigami.Units.gridUnit

                LayoutMirroring.enabled: false  // To preserve spacial layout on RTL
                Layout.alignment: Qt.AlignCenter

                KirigamiComponents.Avatar {
                    KCoreAddons.KUser {
                        id: kuser
                    }
                    source: kuser.faceIconUrl
                    name: kuser.fullName
                    implicitWidth: Kirigami.Units.gridUnit * 4
                    implicitHeight: implicitWidth

                    Layout.row: 1
                    Layout.column: 1
                    Layout.alignment: Qt.AlignCenter
                }

                Repeater {
                    id: channelRepeater

                    model: testOverlay.sinkObject && testOverlay.sinkObject.rawChannels

                    delegate: ToolButton {
                        readonly property bool isPlaying: tester.playingChannels.includes(modelData)
                        readonly property var channelData: {
                            switch (modelData) {
                                case "front-left": return {text: i18nd("kcm_pulseaudio", "Front Left"), row: 0, column: 0, angle: 45};
                                case "front-center": return {text: i18nd("kcm_pulseaudio", "Front Center"), row: 0, column: 1, angle: 90};
                                case "front-right": return {text: i18nd("kcm_pulseaudio", "Front Right"), row: 0, column: 2, angle: 135};
                                case "side-left": return {text: i18nd("kcm_pulseaudio", "Side Left"), row: 1, column: 0, angle: 0};
                                case "side-right": return {text: i18nd("kcm_pulseaudio", "Side Right"), row: 1, column: 2, angle: 180};
                                case "rear-left": return {text: i18nd("kcm_pulseaudio", "Rear Left"), row: 2, column: 0, angle: -45};
                                case "lfe": return {text: i18nd("kcm_pulseaudio", "Subwoofer"), row: 2, column: 1, angle: -90};
                                case "rear-right": return {text: i18nd("kcm_pulseaudio", "Rear Right"), row: 2, column: 2, angle: -135};
                                case "mono" : return {text: i18nd("kcm_pulseaudio", "Mono"), row: 0, column: 1, angle: 90};
                            }
                            // We have a non-standard channel name
                            return {
                                text: modelData,
                                row: 3 + Math.floor(index / 3),
                                // To keep the avatar centered in case of just 1 or 2 non-standard channels
                                column: (channelRepeater.count < 3 && index === channelRepeater.count - 1) ? index + 1 : index % 3,
                                angle: 0
                            }
                        }

                        Layout.row: channelData.row
                        Layout.column: channelData.column
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                        Layout.minimumHeight: Kirigami.Units.gridUnit * 4

                        contentItem: ColumnLayout {
                            spacing: 0

                            Kirigami.Icon {
                                source: ":/kcm/kcm_pulseaudio/icons/audio-speakers-symbolic.svg"
                                isMask: true
                                color: isPlaying ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                                implicitWidth: Kirigami.Units.iconSizes.medium
                                implicitHeight: Kirigami.Units.iconSizes.medium
                                Layout.fillWidth: true
                                Layout.margins: Kirigami.Units.smallSpacing
                                rotation: channelData.angle
                            }

                            Label {
                                text: channelData.text
                                color: isPlaying ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.margins: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignTop
                                wrapMode: Text.WordWrap
                            }
                        }

                        onClicked: {
                            testError.visible = false;
                            tester.testChannel(modelData);
                        }
                    }
                }

                // Spacers for when the left and right columns are empty
                Item {
                    Layout.row: 3
                    Layout.column: 0
                    Layout.fillWidth: true
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Item {
                    Layout.row: 3
                    Layout.column: 2
                    Layout.fillWidth: true
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }

            Label {
                text: i18nd("kcm_pulseaudio", "Click on any speaker to test sound")
                font: Kirigami.Theme.smallFont
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }
}
