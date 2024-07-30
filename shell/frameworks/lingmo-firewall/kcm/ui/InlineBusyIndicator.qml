// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

import QtQuick 2.12
import QtQuick.Controls 2.9 as QQC2

import org.kde.lingmoui 2.10 as LingmoUI

Item {
    id: container

    property alias running: busyIndicator.running
    property int horizontalAlignment: Qt.AlignLeft

    implicitWidth: busyIndicator.implicitWidth
    // Not hiding the container so the layout doesn't shift as it comes and goes

    QQC2.BusyIndicator {
        id: busyIndicator
        anchors {
            left: container.horizontalAlignment === Qt.AlignLeft ? parent.left : undefined
            horizontalCenter: container.horizontalAlignment === Qt.AlignHCenter ? parent.horizontalCenter : undefined
            right: container.horizontalAlignment === Qt.AlignRight ? parent.right : undefined
            verticalCenter: parent.verticalCenter
        }
        width: height
        height: LingmoUI.Units.iconSizes.medium
        running: false
        visible: running
    }
}
