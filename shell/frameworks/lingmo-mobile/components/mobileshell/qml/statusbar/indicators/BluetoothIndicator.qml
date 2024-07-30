/*
    SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.private.mobileshell as MobileShell

LingmoUI.Icon {
    id: connectionIcon

    source: MobileShell.BluetoothInfo.icon

    visible: MobileShell.BluetoothInfo.isVisible
}
