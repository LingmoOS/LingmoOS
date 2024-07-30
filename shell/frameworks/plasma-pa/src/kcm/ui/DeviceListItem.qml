/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.private.volume

ColumnLayout {
    id: delegate
    spacing: Kirigami.Units.smallSpacing
    width: parent.width

    property bool isPlayback: type.substring(0, 4) == "sink"

    readonly property var currentPort: Ports[ActivePortIndex]

    opacity: (Ports.length === 1 && Ports[0].availability == Port.Unavailable) ? 0.5 : 1

    property var pulseObject: model.PulseObject

    property alias portVisible: portBox.visible
    property bool comboBoxLabelsVisible: false

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true
        // To keep the top row the same height as the row below, and stop things moving too much
        // as the KCM loads (and the port/profile ComboBoxes may appear)
        Layout.minimumHeight: channelsButton.implicitHeight

        RadioButton {
            id: defaultButton
            Layout.fillWidth: true
            Layout.leftMargin: LayoutMirroring.enabled ? 0 : Math.round((muteButton.width - defaultButton.indicator.width) / 2)
            Layout.rightMargin: LayoutMirroring.enabled ? Math.round((muteButton.width - defaultButton.indicator.width) / 2) : 0
            spacing: Kirigami.Units.smallSpacing + Math.round((muteButton.width - defaultButton.indicator.width) / 2)
            checked: Default
            visible: delegate.ListView.view.count > 1
            onClicked: Default = true
            text: !currentPort ? Description : i18ndc("kcm_pulseaudio", "label of device items", "%1 (%2)", currentPort.description, Description)

            ToolTip {
                text: parent.text
                visible: parent.visible && parent.contentItem.truncated && labelMouseArea.containsMouse
            }
            MouseArea {
                id: labelMouseArea
                anchors.fill: parent.contentItem
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                z: defaultButton.z + 1
            }
        }

        Label {
            id: soloLabel
            Layout.fillWidth: true
            text: currentPort.description
            visible: delegate.ListView.view.count === 1
            elide: Text.ElideRight

            ToolTip {
                text: defaultButton.text
                visible: parent.visible && soloLabelMouseArea.containsMouse
            }
            MouseArea {
                id: soloLabelMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Item {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        Label {
            id: portLabel
            visible: comboBoxLabelsVisible && portBox.visible
            text: i18nd("kcm_pulseaudio", "Port:")
            Layout.leftMargin: Kirigami.Units.largeSpacing
        }

        ComboBox {
            id: portBox

            readonly property var ports: Ports

            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            visible: portBox.count > 1

            onModelChanged: currentIndex = ActivePortIndex
            currentIndex: ActivePortIndex
            onActivated: ActivePortIndex = index

            onPortsChanged: {
                var items = [];
                for (var i = 0; i < ports.length; ++i) {
                    var port = ports[i];
                    var text;
                    if (port.availability == Port.Unavailable) {
                        if (port.name == "analog-output-speaker" || port.name == "analog-input-microphone-internal") {
                            text = i18ndc("kcm_pulseaudio", "Port is unavailable", "%1 (unavailable)", port.description);
                        } else {
                            text = i18ndc("kcm_pulseaudio", "Port is unplugged", "%1 (unplugged)", port.description);
                        }
                    } else {
                        text = port.description;
                    }
                    items.push(text);
                }
                model = items;
            }

            // Ensure the popup is sufficiently wide to list *full* device names
            // and is correctly positioned to not go off the page
            popup.width: Math.max(implicitWidth, width)
            popup.leftMargin: delegate.ListView.view.Layout.leftMargin
            popup.rightMargin: delegate.ListView.view.Layout.rightMargin
            popup.x: mirrored ? 0 : width - popup.width
        }

        Label {
            id: profileLabel
            visible: comboBoxLabelsVisible && profileBox.visible
            text: i18ndc("kcm_pulseaudio", "@label", "Profile:")
            Layout.leftMargin: Kirigami.Units.largeSpacing
        }

        ComboBox {
            id: profileBox

            readonly property var card: paCardModel.data(paCardModel.indexOfCardNumber(CardIndex), paCardModel.KItemModels.KRoleNames.role("PulseObject"))

            Layout.preferredWidth: Kirigami.Units.gridUnit * 12
            visible: profileBox.count > 1
            textRole: "description"

            model: card ? card.profiles.filter(profile => profile.availability === Profile.Available) : []
            currentIndex: card ? model.indexOf(card.profiles[card.activeProfileIndex]) : -1

            onActivated: index => card.activeProfileIndex = card.profiles.indexOf(model[index])

            // Ensure the popup is sufficiently wide to list *full* device names
            // and is correctly positioned to not go off the page
            popup.width: Math.max(implicitWidth, width)
            popup.leftMargin: delegate.ListView.view.Layout.leftMargin
            popup.rightMargin: delegate.ListView.view.Layout.rightMargin
            popup.x: mirrored ? 0 : width - popup.width
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        MuteButton {
            id: muteButton
            Layout.alignment: Qt.AlignTop | Qt.AlignVCenter
            muted: Muted
            onCheckedChanged: Muted = checked
            toolTipText: !currentPort ? Description : currentPort.description
        }

        GridLayout {
            rows: Math.max(1, channelSliderRepeater.count)
            flow: GridLayout.TopToBottom

            VolumeSlider {
                id: volumeSlider
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignVCenter
                visible: !channelsButton.checked
            }

            Repeater {
                model: channelsButton.checked ? Channels : null

                Label {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    text: i18ndc("kcm_pulseaudio", "Placeholder is channel name", "%1:", modelData)
                }
            }

            Repeater {
                id: channelSliderRepeater
                model: channelsButton.checked ? RawChannels : null

                VolumeSlider {
                    Layout.fillWidth: true

                    value: ChannelVolumes[index]

                    channel: index
                }
            }
        }

        Item {
            Layout.leftMargin: Kirigami.Units.smallSpacing
        }

        Button {
            Layout.alignment: Qt.AlignTop
            id: testButton
            text: i18ndc("kcm_pulseaudio", "Perform an audio test of the device", "Test")
            icon.name: "audio-speakers-symbolic"
            visible: isPlayback
            onClicked: testOverlay.testSink(index);
        }

        Button {
            Layout.alignment: Qt.AlignTop
            id: channelsButton
            text: i18ndc("kcm_pulseaudio", "Show audio channels (e.g. to control left/right audio balance", "Show Channels")
            icon.name: "view-media-equalizer"
            checkable: true

            onClicked: {
                if (checked) {
                    return;
                }

                // When unifying channels again, set all of them to the same value
                // as to not keep an odd difference when adjusting them in unison
                let volumes = ChannelVolumes;
                for (let i = 0, count = RawChannels.length; i < count; ++i) {
                    volumes[i] = Volume;
                }
                // NOTE "ChannelVolumes = volumes" does not work as the
                // AbstractModel does not have the magic JS Array to Qt List
                // conversion stuff and only sees our Array as a QJSValue
                delegate.pulseObject.channelVolumes = volumes;
            }

            // Default to individual mode when a channel has a different volume
            Component.onCompleted: {
                checked = ChannelVolumes.some((volume) => {
                    return volume !== ChannelVolumes[0];
                });
            }
        }
    }

    Kirigami.Separator {
        visible: (delegate.ListView.view.count != 0) && (delegate.ListView.view.count != (index + 1))
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.fillWidth: true
    }
}
