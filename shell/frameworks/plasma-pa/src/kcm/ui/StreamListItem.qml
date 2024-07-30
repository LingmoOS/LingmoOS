/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.private.volume

ColumnLayout {
    id: delegate

    property alias deviceModel: deviceComboBox.model
    readonly property bool isEventStream: Name == "sink-input-by-media-role:event"
    property bool isPlayback

    width: parent ? parent.width : undefined

    RowLayout {
        Layout.fillWidth: true

        ColumnLayout {
            id: delegateColumn
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignHCenter
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    source: {
                        if (IconName.length !== 0) {
                            return IconName
                        }

                        if (delegate.isPlayback) {
                            return "audio-speakers-symbolic"
                        } else {
                            return "audio-input-microphone-symbolic"
                        }
                    }
                }
                Label {
                    id: inputText
                    Layout.fillWidth: true
                    Layout.preferredHeight: deviceComboBox.implicitHeight
                    text: {
                        if (isEventStream) {
                            return i18nd("kcm_pulseaudio", "Notification Sounds");
                        } else if (Client) {
                            if (Client.name === "pipewire-media-session") {
                                return Name;
                            }

                            return i18ndc("kcm_pulseaudio", "label of stream items", "%1: %2", Client.name, Name);
                        } else {
                            return Name;
                        }
                    }
                    elide: Text.ElideRight

                    // Show tooltip on hover when text elided
                    ToolTip {
                        text: parent.text
                        visible: parent.truncated && inputTextMouseArea.containsMouse
                    }
                    MouseArea {
                        id: inputTextMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }

                DeviceComboBox {
                    id: deviceComboBox
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 12
                    visible: !isEventStream && count > 1
                }
            }

            RowLayout {
                MuteButton {
                    Layout.alignment: Qt.AlignTop
                    muted: Muted
                    onCheckedChanged: Muted = checked
                    toolTipText: inputText.text
                }

                VolumeSlider {
                    id: volumeSlider
                    Layout.alignment: Qt.AlignVCenter
                    visible:  HasVolume
                }
            }
        }
    }

    Kirigami.Separator {
        visible: (delegate.ListView.view.count != 0) && (delegate.ListView.view.count != (index + 1))
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        Component.onCompleted: {
            if (isEventStream) {
                visible = Qt.binding(function() {
                    return sinkInputView.count > 0;
                });
            }
        }
    }
}
