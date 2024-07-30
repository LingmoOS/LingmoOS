/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.welcome

Item {
    id: footer

    required property string contentSource

    height: separatorLayout.implicitHeight

    ColumnLayout {
        id: separatorLayout
        anchors.left: footer.left
        anchors.right: footer.right

        spacing: 0

        LingmoUI.Separator {
            Layout.fillWidth: true
        }

        // Not using QQC2.Toolbar so that the window can be
        // dragged from the footer, both appear identical
        LingmoUI.AbstractApplicationHeader {
            Layout.fillWidth: true

            contentItem: Item {
                implicitHeight: footerLoader.implicitHeight + footerLoader.anchors.margins * 2
                implicitWidth: parent.width

                Loader {
                    id: footerLoader
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.smallSpacing

                    source: footer.contentSource
                }
            }
        }
    }
}
