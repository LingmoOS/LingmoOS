/*
    SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.private.volume

Kirigami.Page {
    id: volumeControlsConfigPage
    title: i18n("Volume Controls")

    property var config

    VolumeFeedback {
        id: feedback
    }

    ColumnLayout {
        anchors.fill: parent

        Kirigami.FormLayout {
            wideMode: true

            QQC2.CheckBox {
                checked: config.raiseMaximumVolume
                text: i18n("Raise maximum volume")
                onToggled: { config.raiseMaximumVolume = checked; config.save() }
            }

            QQC2.SpinBox {
                id: volumeStep
                // So it doesn't resize itself when showing a 2 or 3-digit number
                Layout.minimumWidth: Kirigami.Units.gridUnit * 3
                Kirigami.FormData.label: i18n("Volume change step:")
                value: config.volumeStep
                onValueModified: { config.volumeStep = value; config.save() }
                from: 1
                to: 100
                stepSize: 1
                editable: true
                textFromValue: function(value) {
                    return value + "%";
                }
                valueFromText: function(text) {
                    return parseInt(text);
                }
            }

            Item {
                Kirigami.FormData.isSection: true
            }


            QQC2.CheckBox {
                id: volumeFeedback
                checked: config.audioFeedback
                onToggled: { config.audioFeedback = checked; config.save() }
                Kirigami.FormData.label: i18n("Play audio feedback for changes to:")
                text: i18n("Audio volume")
                enabled: feedback.valid
            }

            QQC2.CheckBox {
                id: volumeOsd
                checked: config.volumeOsd
                onToggled: { config.volumeOsd = checked; config.save() }
                Kirigami.FormData.label: i18n("Show visual feedback for changes to:")
                text: i18n("Audio volume")
            }

            QQC2.CheckBox {
                id: micOsd
                checked: config.microphoneSensitivityOsd
                onToggled: { config.microphoneSensitivityOsd = checked; config.save() }
                text: i18n("Microphone sensitivity")
            }

            QQC2.CheckBox {
                id: muteOsd
                checked: config.muteOsd
                onToggled: { config.muteOsd = checked; config.save() }
                text: i18n("Mute state")
            }

            QQC2.CheckBox {
                id: outputChangeOsd
                checked: config.defaultOutputDeviceOsd
                onToggled: { config.defaultOutputDeviceOsd = checked; config.save() }
                text: i18n("Default output device")
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
