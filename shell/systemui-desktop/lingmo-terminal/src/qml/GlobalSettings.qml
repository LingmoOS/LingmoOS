/**
 * New LingmoOS terminal
 * Copyright 2023 LingmoOS Team
 */
import QtQuick 2.0
import Qt.labs.settings 1.0

Settings {
    property int width: 750
    property int height: 500
    property int fontPointSize: 11
    property string fontName: "Ubuntu Mono"
    property int keyboardCursorShape: 0
    property bool blinkingCursor: true

    property var searchUrl: [[qsTr("Bing"),"https://cn.bing.com/search?from=MOZLBR&pc=MOZI&q={KeyWord}"]]
    property var bookmark: []
    property string wordCharacters: ":@-./_~,"
    property var colorschemes: ["Lingmo-Light","Lingmo-Dark","BlackOnLightYellow","BlackOnRandomLight","BlackOnWhite","Tango","Ubuntu","Solarized","SolarizedLight"]
    property string lightcolorScheme: "Lingmo-Light"
    property string darkcolorScheme: "Tango"
    property color lightbackgroundColor : "#F3F4F9"
    property color darkbackgroundColor : "#1C1C1D"

    property double opacity: 0.4
    property bool blur: true
}
