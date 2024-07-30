// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.components 3.0 as PC3
import org.kde.private.mobile.homescreen.folio 1.0 as Folio

PC3.Label {
    id: label
    wrapMode: Text.WordWrap
    maximumLineCount: 2
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignTop
    elide: Text.ElideRight

    font.pointSize: LingmoUI.Theme.defaultFont.pointSize * 0.8
    font.weight: Font.Bold
}
