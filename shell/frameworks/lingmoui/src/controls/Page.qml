/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls as QQC2
import QtQuick.Controls.impl
import org.kde.lingmoui as LingmoUI
import "private" as P

/**
 * Page is a container for all the app pages: everything pushed to the
 * ApplicationWindow's pageStack should be a Page.
 *
 * @see ScrollablePage
 * For content that should be scrollable, such as ListViews, use ScrollablePage instead.
 * @inherit QtQuick.Controls.Page
 */
T.Page {
    id: control

    leftPadding: 0
    
    topPadding: 0
    
    rightPadding: 0
    
    bottomPadding: 0
   
    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            Math.max(contentWidth,
                                     header && header.visible ? header.implicitWidth : 0,
                                     footer && footer.visible ? footer.implicitWidth : 0) + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentHeight + topPadding + bottomPadding
                             + (header && header.visible ? header.implicitHeight + spacing : 0)
                             + (footer && footer.visible ? footer.implicitHeight + spacing : 0))

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
    }
}
