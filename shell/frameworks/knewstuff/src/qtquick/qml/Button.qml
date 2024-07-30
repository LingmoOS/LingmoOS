/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A button which when clicked will open a dialog with a NewStuff.Page at the base
 *
 * This component is equivalent to the old Button
 * @see KNewStuff::Button
 * @since 5.63
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.newstuff as NewStuff

QQC2.Button {
    id: component

    /*
     * The configuration file is not aliased, because then we end up initialising the
     * Engine immediately the Button is shown, which we want to avoid (as that
     * is effectively a phone-home scenario, and causes internet traffic in situations
     * where it would not seem likely that there should be any).
     * If we want, in the future, to add some status display to Button (such as "there
     * are updates to be had" or somesuch, then we can do this, but until that choice is
     * made, let's not)
     */
    /**
     * The configuration file to use for this button
     */
    property string configFile

    /**
     * Set the text that should appear on the button. Will be set as
     * i18nd("knewstuff6", "Download New %1…").
     *
     * @note For the sake of consistency, you should NOT override the text property, just set this one
     */
    property string downloadNewWhat: i18ndc("knewstuff6", "Used to construct the button's label (which will become Download New 'this value'…)", "Stuff")
    text: i18nd("knewstuff6", "Download New %1…", downloadNewWhat)

    /**
     * The view mode of the dialog spawned by this button, which overrides the
     * default one (ViewMode.Tiles). This should be set using the
     * NewStuff.Page.ViewMode enum. Note that ViewMode.Icons has been removed,
     * and asking for it will return ViewMode.Tiles.
     * @see NewStuff.Page.ViewMode
     */
    property int viewMode: NewStuff.Page.ViewMode.Tiles

    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    signal aboutToShowDialog()

    /**
     * The engine which handles the content in this Button
     */
    property NewStuff.Engine engine

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
     * If this is true (default is false), the button will be shown when the Kiosk settings are such
     * that Get Hot New Stuff is disallowed (and any other time enabled is set to false).
     * Usually you would want to leave this alone, but occasionally you may have a reason to
     * leave a button in place that the user is unable to enable.
     */
    property bool visibleWhenDisabled: false

    /**
     * @internal The NewStuff dialog that is opened by the button.
     * Use showDialog() to create and open the dialog.
     */
    property NewStuff.Dialog __ghnsDialog

    /**
     * Show the dialog (same as clicking the button), if allowed by the Kiosk settings
     */
    function showDialog() {
        if (!NewStuff.Settings.allowedByKiosk) {
            // make some noise, because silently doing nothing is a bit annoying
            console.warn("Not allowed by Kiosk");
            return;
        }
        component.aboutToShowDialog();
        // Use this function to open the dialog. It seems roundabout, but this ensures
        // that the dialog is not constructed until we want it to be shown the first time,
        // since it will initialise itself/compile itself when using Loader on the first
        // load and we don't want that until the user explicitly asks for it.
        if (component.__ghnsDialog === null) {
            const dialogComponent = Qt.createComponent("Dialog.qml");
            component.__ghnsDialog = dialogComponent.createObject(component, {
                "configFile": Qt.binding(() => component.configFile),
                "viewMode": Qt.binding(() => component.viewMode),
            });
            dialogComponent.destroy();
        }
        component.__ghnsDialog.open();
        component.engine = component.__ghnsDialog.engine;
    }

    onClicked: showDialog()

    icon.name: "get-hot-new-stuff"
    visible: enabled || visibleWhenDisabled
    enabled: NewStuff.Settings.allowedByKiosk
    onEnabledChanged: {
        // If the user resets this when kiosk has disallowed ghns, force enabled back to false
        if (enabled && !NewStuff.Settings.allowedByKiosk) {
            enabled = false;
        }
    }
}
