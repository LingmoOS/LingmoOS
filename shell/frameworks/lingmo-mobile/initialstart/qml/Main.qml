// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.20 as LingmoUI

import initialstart 1.0 as InitialStart

LingmoUI.ApplicationWindow {
    id: root

    width: 360
    height: 720
    visibility: "Windowed"

    title: i18n("Initial Start")

    pageStack.globalToolBar.style: LingmoUI.ApplicationHeaderStyle.None
    pageStack.columnView.columnResizeMode: LingmoUI.ColumnView.SingleColumn

    pageStack.initialPage: Wizard {}
}
