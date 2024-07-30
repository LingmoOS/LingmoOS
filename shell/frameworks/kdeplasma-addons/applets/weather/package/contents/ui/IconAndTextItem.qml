/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

GridLayout {
    id: iconAndTextRoot

    property alias iconSource: icon.source
    property alias text: label.text
    property bool vertical: false // too bad we cannot make this an enum
    property alias active: icon.active

    readonly property int minimumIconSize: Kirigami.Units.iconSizes.small
    readonly property int iconSize: iconAndTextRoot.vertical ? width : height

    columns: iconAndTextRoot.vertical ? 1 : 2
    rows: iconAndTextRoot.vertical ? 2 : 1

    columnSpacing: 0
    rowSpacing: 0

    Kirigami.Icon {
        id: icon

        readonly property int implicitMinimumIconSize: Math.max(iconSize, minimumIconSize)
        // reset implicit size, so layout in free dimension does not stop at the default one
        implicitWidth: minimumIconSize
        implicitHeight: minimumIconSize

        Layout.fillWidth: iconAndTextRoot.vertical
        Layout.fillHeight: !iconAndTextRoot.vertical
        Layout.minimumWidth: iconAndTextRoot.vertical ? minimumIconSize : implicitMinimumIconSize
        Layout.minimumHeight: iconAndTextRoot.vertical ? implicitMinimumIconSize : minimumIconSize
    }

    Item {
        id: text

        // Otherwise it takes up too much space while loading
        visible: label.text.length > 0

        Layout.fillWidth: iconAndTextRoot.vertical
        Layout.fillHeight: !iconAndTextRoot.vertical
        Layout.minimumWidth: iconAndTextRoot.vertical ? 0 : sizehelper.paintedWidth
        Layout.maximumWidth: iconAndTextRoot.vertical ? Infinity : Layout.minimumWidth

        Layout.minimumHeight: iconAndTextRoot.vertical ? sizehelper.paintedHeight : 0
        Layout.maximumHeight: iconAndTextRoot.vertical ? Layout.minimumHeight : Infinity

        Text {
            id: sizehelper

            font {
                family: label.font.family
                weight: label.font.weight
                italic: label.font.italic
                pixelSize: iconAndTextRoot.vertical ? Kirigami.Units.gridUnit * 2 : 1024 // random "big enough" size - this is used as a max pixelSize by the fontSizeMode
            }
            minimumPixelSize: Math.round(Kirigami.Units.gridUnit / 2)
            fontSizeMode: iconAndTextRoot.vertical ? Text.HorizontalFit : Text.VerticalFit
            wrapMode: Text.NoWrap

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors {
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.smallSpacing
            }
            // These magic values are taken from the digital clock, so that the
            // text sizes here are identical with various clock text sizes
            height: {
                const textHeightScaleFactor = (parent.height > 26) ? 0.7 : 0.9;
                return Math.min (parent.height * textHeightScaleFactor, 3 * Kirigami.Theme.defaultFont.pixelSize);
            }
            visible: false

            // pattern to reserve some constant space TODO: improve and take formatting/i18n into account
            text: "888° X"
            textFormat: Text.PlainText
        }

        PlasmaComponents.Label {
            id: label

            font {
                weight: Font.Normal
                pixelSize: 1024
                pointSize: 0 // we need to unset pointSize otherwise it breaks the Text.Fit size mode
            }
            minimumPixelSize: Math.round(Kirigami.Units.gridUnit / 2)
            fontSizeMode: Text.Fit
            textFormat: Text.PlainText
            wrapMode: Text.NoWrap

            height: 0
            width: 0
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors {
                fill: parent
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.smallSpacing
            }
        }
    }
}
