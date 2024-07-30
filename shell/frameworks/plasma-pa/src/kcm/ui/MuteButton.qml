/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2019 Sefa Eyeoglu <contact@scrumplex.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import org.kde.plasma.private.volume 0.1

QQC2.ToolButton {
    property bool muted: true
    property var toolTipText

    readonly property double maxVolumeValue: PulseAudio.NormalVolume // the applet supports changing this value. We will just assume 65536 (100%)
    readonly property int volumePercent: (Volume / maxVolumeValue) * 100

    icon.name: AudioIcon.forVolume(volumePercent, Muted, isPlayback ? "audio-volume" : "microphone-sensitivity")
    checkable: true
    checked: muted
    onMutedChanged: checked = muted

    QQC2.ToolTip {
        text: muted ? i18ndc("kcm_pulseaudio", "Unmute audio stream", "Unmute %1", toolTipText) : i18ndc("kcm_pulseaudio", "Mute audio stream", "Mute %1", toolTipText)
    }
}
