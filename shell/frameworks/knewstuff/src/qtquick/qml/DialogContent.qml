/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief The contents of the NewStuff.Dialog component
 *
 * This component is equivalent to the old DownloadWidget, but you should consider
 * using NewStuff.Page instead for a more modern style of integration into your
 * application's flow.
 * @see KNewStuff::DownloadWidget
 * @since 5.63
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.ApplicationItem {
    id: component

    property alias downloadNewWhat: newStuffPage.title
    /**
     * The configuration file to use for this button
     */
    property alias configFile: newStuffPage.configFile

    /**
     * The engine which handles the content in this dialog
     */
    property alias engine: newStuffPage.engine

    /**
     * The default view mode of the dialog spawned by this button. This should be
     * set using the NewStuff.Page.ViewMode enum
     * @see NewStuff.Page.ViewMode
     */
    property alias viewMode: newStuffPage.viewMode

    function __showEntryDetails(providerId, entryId) {
        newStuffPage.showEntryDetails(providerId, entryId);
    }

    // Keep in sync with the default sizes in Dialog.qml and dialog.cpp
    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 30

    pageStack.defaultColumnWidth: pageStack.width
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Auto
    pageStack.globalToolBar.canContainHandles: true
    pageStack.initialPage: NewStuff.Page {
        id: newStuffPage

        showUploadAction: false
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
}
