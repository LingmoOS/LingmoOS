/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as PC3
import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg

PC3.ItemDelegate {
    id: __listItem

    /**
     * content: list<Object>
     *
     * The content of this ListItem.
     */
    default property alias content: __innerItem.data

    /**
     * separatorVisible: bool
     *
     * Whether this ListItem displays a separator.
     */
    property bool separatorVisible: true

    /**
     * sectionDelegate: bool
     *
     * Whether this ListItem renders as a section header.
     */
    property bool sectionDelegate: false

    /**
     * containsMouse: bool
     *
     * Whether this ListItem contains the mouse.
     *
     * Alias of Control.hovered.
     */
    readonly property bool containsMouse: hovered

    leftPadding: __background.margins.left
    rightPadding: __background.margins.right
    topPadding: __background.margins.top
    bottomPadding: __background.margins.bottom

    // TODO KF6: `implicitContentWidth, implicitBackgroundWidth, leftInset and rightInset are not available in Controls 2.2 which this component is based on.
    implicitWidth: (contentItem ? contentItem.implicitWidth : 0) + leftPadding + rightPadding

    // TODO KF6: Make this behave more like the normal Control default.
    // Behaving this way for backwards compatibility reasons.
    contentItem: Item {
        id: __innerItem
    }

    background: KSvg.FrameSvgItem {
        id: __background
        imagePath: "widgets/listitem"
        prefix: (__listItem.sectionDelegate ? "section" :
                (__listItem.pressed || __listItem.checked) ? "pressed" : "normal")

        anchors.fill: parent
        visible: __listItem.ListView.view ? __listItem.ListView.view.highlight === null : true

        KSvg.SvgItem {
            svg: KSvg.Svg {
                imagePath: "widgets/listitem"
            }
            elementId: "separator"
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: naturalSize.height
            visible: __listItem.separatorVisible && (__listItem.sectionDelegate || (typeof(index) != "undefined" && index > 0 && !__listItem.checked && !__listItem.pressed))
        }
    }
}
