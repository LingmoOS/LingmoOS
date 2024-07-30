/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A dialog which has a NewStuff.Page at the base
 *
 * This component is equivalent to the old DownloadDialog, but you should consider
 * using NewStuff.Page instead for a more modern style of integration into your
 * application's flow.
 * @see KNewStuff::DownloadDialog
 * @since 5.63
 */

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Window {
    id: component

    // Keep in sync with the implicit sizes in DialogContent.qml and the default
    // size in dialog.cpp
    width: Math.min(Kirigami.Units.gridUnit * 44, Screen.width)
    height: Math.min(Kirigami.Units.gridUnit * 30, Screen.height)

    /**
     * The configuration file to use for this button
     */
    property alias configFile: newStuffPage.configFile

    /**
     * Set the text that should appear as the dialog's title. Will be set as
     * i18nd("knewstuff6", "Download New %1").
     *
     * @default The name defined by your knsrc config file
     * @note For the sake of consistency, you should NOT override the title property, just set this one
     */
    property string downloadNewWhat: engine.name
    title: component.downloadNewWhat.length > 0
        ? i18ndc("knewstuff6", "The dialog title when we know which type of stuff is being requested", "Download New %1", component.downloadNewWhat)
        : i18ndc("knewstuff6", "A placeholder title used in the dialog when there is no better title available", "Download New Stuff")

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

    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    signal aboutToShowDialog()

    /**
     * This forwards the entryEvent from the QtQuick engine
     * @see Engine::entryEvent
     * @since 5.82
     */
    signal entryEvent(var entry, int event)

    property Connections engineConnections: Connections {
        target: component.engine

        function onEntryEvent(entry, event) {
            component.entryEvent(entry, event);
        }
    }

    /**
     * Show the details page for a specific entry.
     * If you call this function before the engine initialisation has been completed,
     * the action itself will be postponed until that has happened.
     * @param providerId The provider ID for the entry you wish to show details for
     * @param entryId The unique ID for the entry you wish to show details for
     * @since 5.79
     */
    function showEntryDetails(providerId, entryId) {
        newStuffPage.__showEntryDetails(providerId, entryId);
    }

    function open() {
        component.visible = true;
    }

    onVisibleChanged: {
        if (visible) {
            newStuffPage.engine.revalidateCacheEntries();
        }
    }

    color: Kirigami.Theme.backgroundColor

    NewStuff.DialogContent {
        id: newStuffPage
        anchors.fill: parent
        downloadNewWhat: component.downloadNewWhat
        Keys.onEscapePressed: event => component.close()
    }

    Component {
        id: uploadPage
        NewStuff.UploadPage {
            objectName: "uploadPage"
            engine: newStuffPage.engine
        }
    }
}
