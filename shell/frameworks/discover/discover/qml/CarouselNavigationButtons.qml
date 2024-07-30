/*
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick

import org.kde.lingmoui as LingmoUI

Item {
    id: root

    // See documentation in CarouselNavigationButton.qml
    required property bool animated
    required property bool atBeginning
    required property bool atEnd

    property real edgeMargin: LingmoUI.Units.gridUnit

    signal decrementCurrentIndex()
    signal incrementCurrentIndex()

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
    LingmoUI.Theme.inherit: false

    anchors.fill: parent

    CarouselNavigationButton {
        LingmoUI.Theme.inherit: true
        LayoutMirroring.enabled: root.LayoutMirroring.enabled

        animated: root.animated
        atBeginning: root.atBeginning
        atEnd: root.atEnd
        edgeMargin: root.edgeMargin
        role: Qt.AlignLeading

        onClicked: {
            root.decrementCurrentIndex();
        }
    }

    CarouselNavigationButton {
        LingmoUI.Theme.inherit: true
        LayoutMirroring.enabled: root.LayoutMirroring.enabled

        animated: root.animated
        atBeginning: root.atBeginning
        atEnd: root.atEnd
        edgeMargin: root.edgeMargin
        role: Qt.AlignTrailing

        onClicked: {
            root.incrementCurrentIndex();
        }
    }
}
