/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import org.kde.plasma.networkmanagement as PlasmaNM

Item {
    readonly property var networkStatus: PlasmaNM.NetworkStatus {}
    readonly property var connectionIcon: PlasmaNM.ConnectionIcon {}
}
