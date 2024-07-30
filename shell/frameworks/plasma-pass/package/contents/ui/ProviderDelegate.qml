// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.plasma.private.plasmapass 1.0

RowLayout {
    id: root

    property ProviderBase provider: null
    property alias icon: providerIcon.source

    PlasmaCore.IconItem {
        id: providerIcon
        width: PlasmaCore.Units.iconSizes.small
        height: PlasmaCore.Units.iconSizes.small
    }

    ColumnLayout {
        PlasmaComponents3.ProgressBar {
            id: timeoutBar

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.valid

            from: 0
            to: root.provider == null ? 0 : root.provider.defaultTimeout
            value: root.provider == null ? 0 : root.provider.timeout
        }

        PlasmaComponents3.Label {
            id: errorLabel

            height: undefined

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.hasError
            text: root.provider != null ? root.provider.error : ""
            wrapMode: Text.WordWrap
        }
    }
}
