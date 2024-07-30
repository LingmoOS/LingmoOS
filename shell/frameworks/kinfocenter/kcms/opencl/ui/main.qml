/*
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2022 Linus Dierheimer <linus@dierheimer.de>
    
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.5

import org.kde.kcmutils as KCM

import org.kde.kinfocenter.private 1.0 as KInfoCenter

KInfoCenter.CommandOutputKCM {
    output: kcm.infoOutputContext
}
