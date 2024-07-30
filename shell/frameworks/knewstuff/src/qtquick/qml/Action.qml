/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief An action which when triggered will open a NewStuff.Dialog or a NewStuff.Page, depending on settings
 *
 * This component is equivalent to the old Button component, but functions in more modern applications
 *
 * The following is a simple example of how to use this Action to show wallpapers from the KDE Store, on a
 * system where Plasma has been installed (and consequently the wallpaper knsrc file is available). This also
 * shows how to make the action push a page to a pageStack rather than opening a dialog:
 *
\code{.qml}
import org.kde.newstuff as NewStuff

NewStuff.Action {
    configFile: "wallpaper.knsrc"
    text: i18n("&Get New Wallpapers…")
    pageStack: applicationWindow().pageStack
    onEntryEvent: function(entry, event) {
        if (event === NewStuff.Entry.StatusChangedEvent) {
            // A entry was installed, updated or removed
        } else if (event === NewStuff.Entry.AdoptedEvent) {
            // The "AdoptionCommand" from the knsrc file was run for the given entry.
            // This should not require refreshing the data for the model
        }
    }
}
\endcode
 *
 * @see NewStuff.Button
 * @since 5.81
 */

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.Action {
    id: component

    /*
     * The configuration file is not aliased, because then we end up initialising the
     * Engine immediately the Action is instantiated, which we want to avoid (as that
     * is effectively a phone-home scenario, and causes internet traffic in situations
     * where it would not seem likely that there should be any).
     * If we want, in the future, to add some status display to the Action (such as "there
     * are updates to be had" or somesuch, then we can do this, but until that choice is
     * made, let's not)
     */
    /**
     * The configuration file to use for the Page created by this action
     */
    property string configFile

    /**
     * The view mode of the page spawned by this action, which overrides the
     * default one (ViewMode.Tiles). This should be set using the
     * NewStuff.Page.ViewMode enum. Note that ViewMode.Icons has been removed,
     * and asking for it will return ViewMode.Tiles.
     * @see NewStuff.Page.ViewMode
     */
    property int viewMode: NewStuff.Page.ViewMode.Tiles

    /**
     * If this is set, the action will push a NewStuff.Page onto this page stack
     * (and request it is made visible if triggered again). If you do not set this
     * property, the action will spawn a NewStuff.Dialog instead.
     * @note If you are building a KCM, set this to your ```kcm``` object.
     */
    property Item pageStack

    /**
     * The engine which handles the content in this Action
     * This will be null until the action has been triggered the first time
     */
    readonly property NewStuff.Engine engine: component._private.engine

    /**
     * This forwards the entry changed event from the QtQuick engine
     * @see Engine::entryEvent
     */
    signal entryEvent(var entry, int event)

    /**
     * If this is true (default is false), the action will be shown when the Kiosk settings are such
     * that Get Hot New Stuff is disallowed (and any other time enabled is set to false).
     * Usually you would want to leave this alone, but occasionally you may have a reason to
     * leave a action in place that the user is unable to enable.
     */
    property bool visibleWhenDisabled: false

    /**
     * The parent window for the dialog created by invoking the action
     *
     * @since 6.1
     */
    property Window transientParent

    /**
    * Show the page/dialog (same as activating the action), if allowed by the Kiosk settings
    */
    function showHotNewStuff() {
        component._private.showHotNewStuff();
    }

    onTriggered: showHotNewStuff()

    icon.name: "get-hot-new-stuff"
    visible: enabled || visibleWhenDisabled
    enabled: NewStuff.Settings.allowedByKiosk
    onEnabledChanged: {
        // If the user resets this when kiosk has disallowed ghns, force enabled back to false
        if (enabled && !NewStuff.Settings.allowedByKiosk) {
            enabled = false;
        }
    }

    readonly property QtObject _private: QtObject {
        property NewStuff.Engine engine: pageItem ? pageItem.engine : null
        // Probably wants to be deleted and cleared if the "mode" changes at runtime...
        property /* NewStuff.Dialog | NewStuff.Page */QtObject pageItem

        readonly property Connections engineConnections: Connections {
            target: component.engine

            function onEntryEvent(entry, event) {
                component.entryEvent(entry, event);
            }
        }

        function showHotNewStuff() {
            if (NewStuff.Settings.allowedByKiosk) {
                if (component.pageStack !== null) {
                    if (component._private.pageItem // If we already have a page created...
                        && (component.pageStack.columnView !== undefined // first make sure that this pagestack is a Kirigami-style one (otherwise just assume we're ok)
                            && component.pageStack.columnView.contains(component._private.pageItem))) // and then check if the page is still in the stack before attempting to...
                    {
                        // ...set the already existing page as the current page
                        component.pageStack.currentItem = component._private.pageItem;
                    } else {
                        component._private.pageItem = newStuffPage.createObject(component);
                        component.pageStack.push(component._private.pageItem);
                    }
                } else {
                    newStuffDialog.open();
                }
            } else {
                // make some noise, because silently doing nothing is a bit annoying
            }
        }

        property Component newStuffPage: Component {
            NewStuff.Page {
                configFile: component.configFile
                viewMode: component.viewMode
            }
        }

        property Item newStuffDialog: Loader {
            // Use this function to open the dialog. It seems roundabout, but this ensures
            // that the dialog is not constructed until we want it to be shown the first time,
            // since it will initialise itself on the first load (which causes it to phone
            // home) and we don't want that until the user explicitly asks for it.
            function open() {
                if (item) {
                    item.open();
                } else {
                    active = true;
                }
            }

            onLoaded: {
                component._private.pageItem = item;
                item.open();
            }

            active: false
            asynchronous: true

            sourceComponent: NewStuff.Dialog {
                transientParent: component.transientParent
                configFile: component.configFile
                viewMode: component.viewMode
            }
        }
    }
}
