/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>
*/

import QtQuick 2.5

import org.kde.kcmutils as KCM

import org.kde.kinfocenter.private 1.0 as KInfoCenter

KInfoCenter.CommandOutputKCM {
    output: kcm.infoOutputContext
}
