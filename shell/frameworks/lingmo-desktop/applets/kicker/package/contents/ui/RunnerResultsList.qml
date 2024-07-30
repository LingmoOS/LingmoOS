/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.plasmoid

FocusScope {
    width: runnerMatches.width + vertLine.width + vertLine.anchors.leftMargin + runnerMatches.anchors.leftMargin
    height: parent.height

    signal keyNavigationAtListEnd

    property alias currentIndex: runnerMatches.currentIndex
    property alias count: runnerMatches.count
    property alias containsMouse: runnerMatches.containsMouse

    Accessible.name: header.text
    Accessible.role: Accessible.MenuItem

    readonly property bool firstVisible: parent.visibleChildren[0] === this
    KSvg.SvgItem {
        id: vertLine

        anchors.left: parent.left
        anchors.leftMargin: firstVisible ? 0 : LingmoUI.Units.smallSpacing

        width: firstVisible ? 0 : lineSvg.vertLineWidth
        height: parent.height

        visible: !firstVisible

        svg: lineSvg
        elementId: "vertical-line"
    }

    LingmoComponents3.Label {
        id: header

        anchors.left: vertLine.right

        width: runnerMatches.width
        height: runnerMatches.itemHeight + LingmoUI.Units.smallSpacing

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        textFormat: Text.PlainText
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        font.weight: Font.Bold

        text: (runnerMatches.model !== null) ? runnerMatches.model.name : ""
    }

    ItemListView {
        id: runnerMatches

        anchors.top: Plasmoid.configuration.alignResultsToBottom ? undefined : header.bottom
        anchors.bottom: Plasmoid.configuration.alignResultsToBottom ? parent.bottom : undefined
        anchors.bottomMargin: (firstVisible && anchors.bottom !== undefined) ? searchField.height + (2 * LingmoUI.Units.smallSpacing) : undefined
        anchors.left: vertLine.right
        anchors.leftMargin: firstVisible ? 0 : LingmoUI.Units.smallSpacing

        height: {
            var listHeight = ((firstVisible
                ? rootList.height : runnerColumns.height) - header.height);

            if (model && model.count) {
                return Math.min(favoriteSystemActions.height + favoriteApps.height - header.height, model.count * itemHeight);
            }

            return listHeight;
        }

        focus: true

        iconsEnabled: true
        keyNavigationWraps: (index != 0)

        resetOnExitDelay: 0

        model: runnerModel.modelForRow(index)

        onCountChanged: {
            if (firstVisible && searchField.focus) {
                currentIndex = 0;
            }
        }
    }

    Component.onCompleted: {
        runnerMatches.keyNavigationAtListEnd.connect(keyNavigationAtListEnd);
    }
}
