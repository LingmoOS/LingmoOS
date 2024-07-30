/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import org.kde.lingmo.networkmanagement as LingmoNM

Item {
    readonly property var networkStatus: LingmoNM.NetworkStatus {}
    readonly property var connectionIcon: LingmoNM.ConnectionIcon {}
}
