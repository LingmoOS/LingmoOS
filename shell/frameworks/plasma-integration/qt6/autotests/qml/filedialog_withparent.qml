/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Window
import QtQuick.Dialogs

Window {
    x: 100
    y: 100
    width: 100
    height: 100

    FileDialog {
        id: fileDialog
        Component.onCompleted: visible = true
    }
}
