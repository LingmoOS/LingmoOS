/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
//for TabBar.*
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.qqc2desktopstyle.private as StylePrivate

T.TabButton {
    id: controlRoot

    //Some qstyles like fusion don't have correct pixel metrics here and just return 0
    implicitWidth: Math.max(styleitem.implicitWidth, textMetrics.width + Kirigami.Units.gridUnit * 2)
    implicitHeight: styleitem.implicitHeight || Kirigami.Units.gridUnit * 2
    baselineOffset: contentItem.y + contentItem.baselineOffset

    //This width: is important to make the tabbar internals not assume
    //all tabs have the same width
    width: implicitWidth
    padding: 0

    hoverEnabled: true

    contentItem: Item {}

    Kirigami.MnemonicData.enabled: enabled && visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: text
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.Kirigami.MnemonicData.sequence
        onActivated: controlRoot.checked = true;
    }
    background: StylePrivate.StyleItem {
        id: styleitem

        TextMetrics {
            id: textMetrics
            font: Kirigami.Theme.defaultFont
            text: controlRoot.text
        }

        control: controlRoot
        anchors.fill: parent
        elementType: "tab"
        paintMargins: 0

        readonly property T.TabBar tabBar: controlRoot.TabBar.tabBar

        property string orientation: controlRoot.TabBar.position === TabBar.Header ? "Top" : "Bottom"
        property string selectedpos: {
            if (tabBar) {
                switch (tabBar.currentIndex) {
                case (controlRoot.TabBar.index + 1): return "next";
                case (controlRoot.TabBar.index - 1): return "previous";
                }
            }
            return "";
        }
        property string tabpos: {
            if (tabBar) {
                if (tabBar.count === 1) {
                    return "only";
                }
                switch (controlRoot.TabBar.index) {
                case 0: return "beginning";
                case (tabBar.count - 1): return "end";
                }
            }
            return "middle";
        }

        properties: {
            "icon": controlRoot.display !== T.AbstractButton.TextOnly
                ? (controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source) : null,
            "iconColor": Qt.colorEqual(controlRoot.icon.color, "transparent") ? Kirigami.Theme.textColor : controlRoot.icon.color,
            "iconWidth": controlRoot.icon.width,
            "iconHeight": controlRoot.icon.height,

            "hasFrame" : true,
            "orientation": orientation,
            "tabpos": tabpos,
            "selectedpos": selectedpos,
        }

        enabled: controlRoot.enabled
        selected: controlRoot.checked
        text: controlRoot.Kirigami.MnemonicData.mnemonicLabel
        hover: controlRoot.hovered
        hasFocus: controlRoot.activeFocus
    }
}
