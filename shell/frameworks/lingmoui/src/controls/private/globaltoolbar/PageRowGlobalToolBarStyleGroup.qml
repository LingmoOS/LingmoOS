/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

QtObject {
    id: globalToolBar
    property int style: LingmoUI.ApplicationHeaderStyle.None

    readonly property int actualStyle: {
        if (style === LingmoUI.ApplicationHeaderStyle.Auto) {
            if (!LingmoUI.Settings.isMobile) {
                return LingmoUI.ApplicationHeaderStyle.ToolBar
            } else if (root.wideMode) {
                return LingmoUI.ApplicationHeaderStyle.Titles
            } else {
                return LingmoUI.ApplicationHeaderStyle.Breadcrumb
            }
        }
        return style;
    }

    /** @property lingmoui::ApplicationHeaderStyle::NavigationButtons */
    property int showNavigationButtons: (!LingmoUI.Settings.isMobile || Qt.platform.os === "ios")
        ? (LingmoUI.ApplicationHeaderStyle.ShowBackButton | LingmoUI.ApplicationHeaderStyle.ShowForwardButton)
        : LingmoUI.ApplicationHeaderStyle.NoNavigationButtons
    property bool separatorVisible: true
    //Unfortunately we can't access pageRow.globalToolbar.LingmoUI.Theme directly in a declarative way
    property int colorSet: LingmoUI.Theme.Header
    // whether or not the header should be
    // "pushed" back when scrolling using the
    // touch screen
    property bool hideWhenTouchScrolling: false
    /**
     * If true, when any kind of toolbar is shown, the drawer handles will be shown inside the toolbar, if they're present
     */
    property bool canContainHandles: true
    property int toolbarActionAlignment: Qt.AlignRight
    property int toolbarActionHeightMode: LingmoUI.ToolBarLayout.ConstrainIfLarger

    property int minimumHeight: 0
    // FIXME: Figure out the exact standard size of a Toolbar
    property int preferredHeight: (actualStyle === LingmoUI.ApplicationHeaderStyle.ToolBar
                    ? LingmoUI.Units.iconSizes.medium
                    : LingmoUI.Units.gridUnit * 1.8) + LingmoUI.Units.smallSpacing * 2
    property int maximumHeight: preferredHeight

    // Sets the minimum leading padding for the title in a page header
    property int titleLeftPadding: LingmoUI.Units.gridUnit
}
