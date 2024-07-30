/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.lingmoui 2.20 as LingmoUI

ColumnLayout {
    id: root

    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    LingmoComponents3.Label {
        text: Qt.formatTime(timeSource.data["Local"]["DateTime"], Qt.locale(), Locale.ShortFormat)
        textFormat: Text.PlainText
        style: root.softwareRendering ? Text.Outline : Text.Normal
        styleColor: root.softwareRendering ? LingmoUI.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        font.pointSize: Math.round(LingmoUI.Theme.defaultFont.pointSize * 4.8)
        Layout.alignment: Qt.AlignHCenter
    }
    LingmoComponents3.Label {
        text: Qt.formatDate(timeSource.data["Local"]["DateTime"], Qt.locale(), Locale.LongFormat)
        textFormat: Text.PlainText
        style: root.softwareRendering ? Text.Outline : Text.Normal
        styleColor: root.softwareRendering ? LingmoUI.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        font.pointSize: Math.round(LingmoUI.Theme.defaultFont.pointSize * 2.4)
        Layout.alignment: Qt.AlignHCenter
    }
    P5Support.DataSource {
        id: timeSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 1000
    }
}
