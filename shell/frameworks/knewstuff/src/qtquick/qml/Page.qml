/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A Kirigami.Page component used for managing KNS entries
 *
 * This component is functionally equivalent to the old DownloadDialog
 * @see KNewStuff::DownloadDialog
 * @since 5.63
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

import "private" as Private
import "private/entrygriddelegates" as EntryGridDelegates

KCMUtils.GridViewKCM {
    id: root

    /**
     * @brief The configuration file which describes the application (knsrc)
     *
     * The format and location of this file is found in the documentation for
     * KNS3::DownloadDialog
     */
    property alias configFile: newStuffEngine.configFile

    readonly property alias engine: newStuffEngine

    /**
     * Whether or not to show the Upload... context action
     * Usually this will be bound to the engine's property which usually defines
     * this, but you can override it programmatically by setting it here.
     * @since 5.85
     * @see KNSCore::Engine::uploadEnabled
     */
    property alias showUploadAction: uploadAction.visible

    /**
     * Show the details page for a specific entry.
     * If you call this function before the engine initialisation has been completed,
     * the action itself will be postponed until that has happened.
     * @param providerId The provider ID for the entry you wish to show details for
     * @param entryId The unique ID for the entry you wish to show details for
     * @since 5.79
     */
    function showEntryDetails(providerId, entryId) {
        _showEntryDetailsThrottle.enabled = true;
        _showEntryDetailsThrottle.entry = newStuffEngine. __createEntry(providerId, entryId);
        if (newStuffEngine.busyState === NewStuff.Engine.Initializing) {
            _showEntryDetailsThrottle.queryWhenInitialized = true;
        } else {
            _showEntryDetailsThrottle.requestDetails();
        }
    }

    // Helper for loading and showing entry details
    Connections {
        id: _showEntryDetailsThrottle
        target: newStuffModel.engine
        enabled: false

        property var entry
        property bool queryWhenInitialized: false

        function requestDetails() {
            newStuffEngine.updateEntryContents(entry);
            queryWhenInitialized = false;
        }

        function onBusyStateChanged() {
           if (queryWhenInitialized && newStuffEngine.busyState !== NewStuff.Engine.Initializing) {
                requestDetails();
                queryWhenInitialized = false;
           }
        }

        function onSignalEntryEvent(changedEntry, event) {
            if (event === NewStuff.Engine.DetailsLoadedEvent && changedEntry === entry) { // only uniqueId and providerId are checked for equality
                enabled = false;
                pageStack.push(detailsPage, {
                    newStuffModel,
                    providerId: changedEntry.providerId,
                    entry: changedEntry,
                });
            }
        }
    }

    Connections {
        id: _restoreSearchState

        target: pageStack
        enabled: false

        function onCurrentIndexChanged() {
            if (pageStack.currentIndex === 0) {
                newStuffEngine.restoreSearch();
                _restoreSearchState.enabled = false;
            }
        }
    }

    property string uninstallLabel: i18ndc("knewstuff6", "Request uninstallation of this item", "Uninstall")
    property string useLabel: engine.useLabel

    property int viewMode: Page.ViewMode.Tiles

    // TODO KF7: remove Icons
    enum ViewMode {
        Tiles,
        Icons,
        Preview
    }

    // Otherwise the first item will be focused, see BUG: 424894
    Component.onCompleted: {
        view.currentIndex = -1;
    }

    title: newStuffEngine.name

    headerPaddingEnabled: false
    header: Kirigami.InlineMessage {
        readonly property bool riskyContent: newStuffEngine.contentWarningType === NewStuff.Engine.Executables
        visible: !loadingOverlay.visible
        type: riskyContent ? Kirigami.MessageType.Warning : Kirigami.MessageType.Information
        position: Kirigami.InlineMessage.Position.Header
        text: riskyContent
            ? xi18nd("knewstuff6", "Use caution when accessing user-created content shown here, as it may contain executable code that hasn't been tested by KDE or your distributor for safety, stability, or quality.")
            : i18nd("knewstuff6", "User-created content shown here hasn't been tested by KDE or your distributor for functionality or quality.")
    }

    NewStuff.Engine {
        id: newStuffEngine
    }

    NewStuff.QuestionAsker {
        parent: root.QQC2.Overlay.overlay
    }

    Private.ErrorDisplayer {
        engine: newStuffEngine
        active: root.isCurrentPage
    }

    QQC2.ActionGroup { id: viewFilterActionGroup }
    QQC2.ActionGroup { id: viewSortingActionGroup }

    actions: [
        Kirigami.Action {
            visible: newStuffEngine.needsLazyLoadSpinner
            displayComponent: QQC2.BusyIndicator {
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium
            }
        },

        Kirigami.Action {
            text: {
                if (newStuffEngine.filter === 0) {
                    return i18nd("knewstuff6", "Everything");
                } else if (newStuffEngine.filter === 1) {
                    return i18nd("knewstuff6", "Installed");
                } else if (newStuffEngine.filter === 2) {
                    return i18nd("knewstuff6", "Updateable");
                } else {
                    // then it's ExactEntryId and we want to probably just ignore that
                }
            }
            checkable: false
            icon.name: {
                if (newStuffEngine.filter === 0) {
                    return "package-available"
                } else if (newStuffEngine.filter === 1) {
                    return "package-installed-updated"
                } else if (newStuffEngine.filter === 2) {
                    return "package-installed-outdated"
                } else {
                    // then it's ExactEntryId and we want to probably just ignore that
                }
            }

            Kirigami.Action {
                icon.name: "package-available"
                text: i18ndc("knewstuff6", "List option which will set the filter to show everything", "Show All Entries")
                checkable: true
                checked: newStuffEngine.filter === 0
                onTriggered: source => {
                    newStuffEngine.filter = 0;
                }
                QQC2.ActionGroup.group: viewFilterActionGroup
            }

            Kirigami.Action {
                icon.name: "package-installed-updated"
                text: i18ndc("knewstuff6", "List option which will set the filter so only installed items are shown", "Show Only Installed Entries")
                checkable: true
                checked: newStuffEngine.filter === 1
                onTriggered: source => {
                    newStuffEngine.filter = 1;
                }
                QQC2.ActionGroup.group: viewFilterActionGroup
            }

            Kirigami.Action {
                icon.name: "package-installed-outdated"
                text: i18ndc("knewstuff6", "List option which will set the filter so only installed items with updates available are shown", "Show Only Updateable Entries")
                checkable: true
                checked: newStuffEngine.filter === 2
                onTriggered: source => {
                    newStuffEngine.filter = 2;
                }
                QQC2.ActionGroup.group: viewFilterActionGroup
            }
        },

        Kirigami.Action {
            text: {
                if (newStuffEngine.sortOrder === 0) {
                    return i18nd("knewstuff6", "Recent");
                } else if (newStuffEngine.sortOrder === 1) {
                    return i18nd("knewstuff6", "Alphabetical");
                } else if (newStuffEngine.sortOrder === 2) {
                    return i18nd("knewstuff6", "Rating");
                } else if (newStuffEngine.sortOrder === 3) {
                    return i18nd("knewstuff6", "Downloads");
                } else {
                }
            }
            checkable: false
            icon.name: {
                if (newStuffEngine.sortOrder === 0) {
                    return "change-date-symbolic";
                } else if (newStuffEngine.sortOrder === 1) {
                    return "sort-name";
                } else if (newStuffEngine.sortOrder === 2) {
                    return "rating";
                } else if (newStuffEngine.sortOrder === 3) {
                    return "download";
                } else {
                }
            }

            Kirigami.Action {
                icon.name: "change-date-symbolic"
                text: i18ndc("knewstuff6", "List option which will set the sort order to based on when items were most recently updated", "Show Most Recent First")
                checkable: true
                checked: newStuffEngine.sortOrder === 0
                onTriggered: source => {
                    newStuffEngine.sortOrder = 0;
                }
                QQC2.ActionGroup.group: viewSortingActionGroup
            }

            Kirigami.Action {
                icon.name: "sort-name"
                text: i18ndc("knewstuff6", "List option which will set the sort order to be alphabetical based on the name", "Sort Alphabetically By Name")
                checkable: true
                checked: newStuffEngine.sortOrder === 1
                onTriggered: source => {
                    newStuffEngine.sortOrder = 1;
                }
                QQC2.ActionGroup.group: viewSortingActionGroup
            }

            Kirigami.Action {
                icon.name: "rating"
                text: i18ndc("knewstuff6", "List option which will set the sort order to based on user ratings", "Show Highest Rated First")
                checkable: true
                checked: newStuffEngine.sortOrder === 2
                onTriggered: source => {
                    newStuffEngine.sortOrder = 2;
                }
                QQC2.ActionGroup.group: viewSortingActionGroup
            }

            Kirigami.Action {
                icon.name: "download"
                text: i18ndc("knewstuff6", "List option which will set the sort order to based on number of downloads", "Show Most Downloaded First")
                checkable: true
                checked: newStuffEngine.sortOrder === 3
                onTriggered: source => {
                    newStuffEngine.sortOrder = 3;
                }
                QQC2.ActionGroup.group: viewSortingActionGroup
            }
        },

        Kirigami.Action {
            id: uploadAction

            text: i18nd("knewstuff6", "Upload…")
            tooltip: i18nd("knewstuff6", "Learn how to add your own hot new stuff to this list")
            icon.name: "upload-media"
            visible: newStuffEngine.uploadEnabled

            onTriggered: source => {
                pageStack.push(uploadPage);
            }
        },

        Kirigami.Action {
            text: i18nd("knewstuff6", "Go to…")
            icon.name: "go-next"
            id: searchModelActions
            visible: children.length > 0
        },

        Kirigami.Action {
            text: i18nd("knewstuff6", "Search…")
            icon.name: "system-search"
            displayHint: Kirigami.DisplayHint.KeepVisible

            displayComponent: Kirigami.SearchField {
                id: searchField

                enabled: engine.isValid
                focusSequence: "Ctrl+F"
                placeholderText: i18nd("knewstuff6", "Search…")
                text: newStuffEngine.searchTerm

                onAccepted: {
                    newStuffEngine.searchTerm = searchField.text;
                }

                Component.onCompleted: {
                    if (!Kirigami.InputMethod.willShowOnActive) {
                        forceActiveFocus();
                    }
                }
            }
        }
    ]

    Instantiator {
        id: searchPresetInstatiator

        model: newStuffEngine.searchPresetModel

        Kirigami.Action {
            required property int index

            text: model.displayName
            icon.name: model.iconName

            onTriggered: source => {
                const curIndex = newStuffEngine.searchPresetModel.index(index, 0);
                newStuffEngine.searchPresetModel.loadSearch(curIndex);
            }
        }

        onObjectAdded: (index, object) => {
            searchModelActions.children.push(object);
        }
    }

    Connections {
        target: newStuffEngine.searchPresetModel

        function onModelReset() {
            searchModelActions.children = [];
        }
    }

    footer: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        visible: visibleChildren.length > 0
        height: visible ? implicitHeight : 0

        QQC2.Label {
            visible: categoriesCombo.count > 2
            text: i18nd("knewstuff6", "Category:")
        }

        QQC2.ComboBox {
            id: categoriesCombo

            Layout.fillWidth: true

            visible: count > 2
            model: newStuffEngine.categories
            textRole: "displayName"

            onCurrentIndexChanged: {
                newStuffEngine.categoriesFilter = model.data(model.index(currentIndex, 0), NewStuff.CategoriesModel.NameRole);
            }
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignRight

            text: i18nd("knewstuff6", "Contribute your own…")
            icon.name: "upload-media"
            visible: newStuffEngine.uploadEnabled && !uploadAction.visible

            onClicked: {
                pageStack.push(uploadPage);
            }
        }
    }

    view.model: NewStuff.ItemsModel {
        id: newStuffModel

        engine: newStuffEngine
    }

    NewStuff.DownloadItemsSheet {
        id: downloadItemsSheet

        parent: root.QQC2.Overlay.overlay

        onItemPicked: (entry, downloadItemId) => {
            newStuffModel.engine.install(entry, downloadItemId);
        }
    }

    view.implicitCellWidth: switch (root.viewMode) {
        case Page.ViewMode.Preview:
            return Kirigami.Units.gridUnit * 25;

        case Page.ViewMode.Tiles:
        case Page.ViewMode.Icons:
        default:
            return Kirigami.Units.gridUnit * 30;
    }

    view.implicitCellHeight: switch (root.viewMode) {
        case Page.ViewMode.Preview:
            return Kirigami.Units.gridUnit * 25;

        case Page.ViewMode.Tiles:
        case Page.ViewMode.Icons:
        default:
            return Math.round(view.implicitCellWidth / 3);
    }

    view.delegate: switch (root.viewMode) {
        case Page.ViewMode.Preview:
            return bigPreviewDelegate;

        case Page.ViewMode.Tiles:
        case Page.ViewMode.Icons:
        default:
            return tileDelegate;
    }

    Component {
        id: bigPreviewDelegate

        EntryGridDelegates.BigPreviewDelegate { }
    }

    Component {
        id: tileDelegate

        EntryGridDelegates.TileDelegate {
            useLabel: root.useLabel
            uninstallLabel: root.uninstallLabel
        }
    }

    Component {
        id: detailsPage

        NewStuff.EntryDetails { }
    }

    Component {
        id: uploadPage

        NewStuff.UploadPage {
            engine: newStuffEngine
        }
    }

    Item {
        id: loadingOverlay

        anchors.fill: parent

        opacity: newStuffEngine.isLoading && !newStuffEngine.needsLazyLoadSpinner ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }

        visible: opacity > 0

        Rectangle {
            anchors.fill: parent
            color: Kirigami.Theme.backgroundColor
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            text: newStuffEngine.busyMessage
        }
    }
}
