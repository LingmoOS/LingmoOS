// SPDX-FileCopyrightText: 2023-2024 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui 2.20 as LingmoUI

Loader {
    id: root
    asynchronous: true

    property var lockScreenState

    sourceComponent: {
        if (lockScreenState.isFingerprintSupported) {
            return fingerprintIcon;
        } else {
            return scrollUpIcon;
        }
    }

    Component {
        id: scrollUpIcon

        LingmoUI.Icon {
            implicitWidth: LingmoUI.Units.iconSizes.small
            implicitHeight: LingmoUI.Units.iconSizes.small

            LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
            source: "arrow-up"
        }
    }

    Component {
        id: fingerprintIcon

        LingmoUI.Icon {
            source: 'fingerprint-symbolic'
            implicitWidth: LingmoUI.Units.iconSizes.medium
            implicitHeight: LingmoUI.Units.iconSizes.medium

            LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: LingmoUI.Units.gridUnit * 2 + flickable.position * 0.5
        }
    }
}
