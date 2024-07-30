// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.lingmoui 2.19 as LingmoUI


LingmoUI.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "Overview")
    minimumWidth: LingmoUI.Settings.isMobile ? 0 : LingmoUI.Units.gridUnit * 22
    minimumHeight: LingmoUI.Settings.isMobile ? 0 : LingmoUI.Units.gridUnit * 22

    pageStack.initialPage: ListPage {}
    pageStack.defaultColumnWidth: root.width // show single page
}
