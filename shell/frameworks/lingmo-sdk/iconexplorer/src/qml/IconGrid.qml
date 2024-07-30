/*
    SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.lingmoui 2.8 as LingmoUI

GridView {
    id: iconGrid

    topMargin: LingmoUI.Units.gridUnit
    leftMargin: LingmoUI.Units.gridUnit
    rightMargin: LingmoUI.Units.gridUnit
    bottomMargin: LingmoUI.Units.gridUnit

    cellWidth: iconSize + Math.round(LingmoUI.Units.gridUnit * 1.5)
    cellHeight: iconSize + Math.round(LingmoUI.Units.gridUnit * 3.5)

    cacheBuffer: 20
    highlightMoveDuration: 0
    model: proxyModel
    currentIndex: proxyModel.currentIndex

    highlight: Item {}

    delegate: IconGridDelegate {}

    QQC2.BusyIndicator {
        running: iconModel.loading
        visible: running
        anchors.centerIn: parent
        width: LingmoUI.Units.gridUnit * 8
        height: width
    }
    Component.onCompleted: {
        currentItem.setAsPreview()
    }
}
