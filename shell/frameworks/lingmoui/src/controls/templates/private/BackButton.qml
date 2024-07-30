/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

QQC2.ToolButton {
    id: button

    icon.name: (LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic")

    enabled: {
        const pageStack = applicationWindow().pageStack;

        if (pageStack.layers.depth > 1) {
            return true;
        }

        if (pageStack.depth > 1) {
            if (pageStack.currentIndex > 0) {
                return true;
            }

            const view = pageStack.columnView;
            if (LayoutMirroring.enabled) {
                return view.contentWidth - view.width < view.contentX
            } else {
                return view.contentX > 0;
            }
        }

        return false;
    }

    // The gridUnit wiggle room is used to not flicker the button visibility during an animated resize for instance due to a sidebar collapse
    visible: {
        const pageStack = applicationWindow().pageStack;
        const showNavButtons = globalToolBar?.showNavigationButtons ?? LingmoUI.ApplicationHeaderStyle.NoNavigationButtons;
        return pageStack.layers.depth > 1 || (pageStack.contentItem.contentWidth > pageStack.width + LingmoUI.Units.gridUnit && (showNavButtons & LingmoUI.ApplicationHeaderStyle.ShowBackButton));
    }

    onClicked: {
        applicationWindow().pageStack.goBack();
    }

    text: qsTr("Navigate Back")
    display: QQC2.ToolButton.IconOnly

    QQC2.ToolTip {
        visible: button.hovered
        text: button.text
        delay: LingmoUI.Units.toolTipDelay
        timeout: 5000
        y: button.height
    }
}
