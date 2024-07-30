/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQml
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

AbstractPageHeader {
    id: root

    implicitWidth: layout.implicitWidth + LingmoUI.Units.smallSpacing * 2
    implicitHeight: Math.max(titleLoader.implicitHeight, toolBar.implicitHeight) + LingmoUI.Units.smallSpacing * 2

    MouseArea {
        anchors.fill: parent
        onPressed: mouse => {
            page.forceActiveFocus()
            mouse.accepted = false
        }
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        anchors.rightMargin: LingmoUI.Units.smallSpacing
        spacing: LingmoUI.Units.smallSpacing

        Loader {
            id: titleLoader

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: item?.Layout.fillWidth ?? false
            Layout.minimumWidth: item?.Layout.minimumWidth ?? -1
            Layout.preferredWidth: item?.Layout.preferredWidth ?? -1
            Layout.maximumWidth: item?.Layout.maximumWidth ?? -1

            // Don't load async to prevent jumpy behaviour on slower devices as it loads in.
            // If the title delegate really needs to load async, it should be its responsibility to do it itself.
            asynchronous: false
            sourceComponent: page?.titleDelegate ?? null
        }

        LingmoUI.ActionToolBar {
            id: toolBar

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: actions.length > 0
            alignment: pageRow?.globalToolBar.toolbarActionAlignment ?? Qt.AlignRight
            heightMode: pageRow?.globalToolBar.toolbarActionHeightMode ?? LingmoUI.ToolBarLayout.ConstrainIfLarger

            actions: page?.actions ?? []
        }
    }
}
