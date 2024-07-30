/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A Kirigami.Page component used for displaying the details for a single entry
 *
 * This component is equivalent to the details view in the old DownloadDialog
 * @see KNewStuff::DownloadDialog
 * @since 5.63
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.newstuff as NewStuff

import "private" as Private

KCMUtils.SimpleKCM {
    id: component

    property NewStuff.ItemsModel newStuffModel
    property var entry

    property string name
    property var author
    property alias shortSummary: shortSummaryItem.text
    property alias summary: summaryItem.text
    property alias previews: screenshotsItem.screenshotsModel
    property string homepage
    property string donationLink
    property int status
    property int commentsCount
    property int rating
    property int downloadCount
    property var downloadLinks
    property string providerId
    property int entryType

    Component.onCompleted: {
        updateContents();
        newStuffModel.engine.updateEntryContents(component.entry);
    }

    Connections {
        target: newStuffModel
        function onEntryChanged(changedEntry) {
            if (entry === changedEntry) {
                updateContents();
            }
        }
    }

    function updateContents() {
        component.providerId = entry.providerId;
        component.status = entry.status;

        component.author = entry.author;
        component.name = entry.name;
        component.shortSummary = entry.shortSummary;
        component.summary = entry.summary;
        component.homepage = entry.homepage;
        component.donationLink = entry.donationLink;
        component.status = entry.status;
        component.commentsCount = entry.numberOfComments;
        component.rating = entry.rating;
        component.downloadCount = entry.downloadCount;

        const modelIndex = newStuffModel.index(newStuffModel.indexOfEntry(entry), 0);
        component.previews = newStuffModel.data(modelIndex, NewStuff.ItemsModel.PreviewsRole);
        component.downloadLinks = newStuffModel.data(modelIndex, NewStuff.ItemsModel.DownloadLinksRole);

    }

    NewStuff.DownloadItemsSheet {
        id: downloadItemsSheet

        parent: component.QQC2.Overlay.overlay

        onItemPicked: (entry, downloadItemId, downloadName) => {
            const entryName = component.newStuffModel.data(component.newStuffModel.index(downloadItemId, 0), NewStuff.ItemsModel.NameRole);
            applicationWindow().showPassiveNotification(i18ndc("knewstuff6", "A passive notification shown when installation of an item is initiated", "Installing %1 from %2", downloadName, entryName), 1500);
            component.newStuffModel.engine.install(component.entry, downloadItemId);
        }
    }

    Private.ErrorDisplayer {
        engine: component.newStuffModel.engine
        active: component.isCurrentPage
    }

    NewStuff.Author {
        id: entryAuthor
        engine: component.newStuffModel.engine
        providerId: component.providerId
        username: author.name
    }

    title: i18ndc("knewstuff6", "Combined title for the entry details page made of the name of the entry, and the author's name", "%1 by %2", component.name, entryAuthor.name)

    actions: [
        Kirigami.Action {
            text: component.downloadLinks.length === 1
                ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install")
                : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…")

            icon.name: "install"
            onTriggered: source => {
                if (component.downloadLinks.length === 1) {
                    newStuffModel.engine.install(component.entry, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = component.downloadLinks;
                    downloadItemsSheet.entry = component.index;
                    downloadItemsSheet.open();
                }
            }
            enabled: component.status === NewStuff.Entry.Downloadable || component.status === NewStuff.Entry.Deleted
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "update-none"
            onTriggered: source => newStuffModel.update(component.entry, NewStuff.ItemsModel.AutoDetectLinkId)
            enabled: component.status === NewStuff.Entry.Updateable
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request uninstallation of this item", "Uninstall")
            icon.name: "edit-delete"
            onTriggered: source => newStuffModel.engine.uninstall(component.entry)
            enabled: component.status === NewStuff.Entry.Installed || component.status === NewStuff.Entry.Updateable
            visible: enabled
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.AbstractCard {
            id: statusCard

            readonly property string message: {
                switch (component.status) {
                case NewStuff.Entry.Downloadable:
                case NewStuff.Entry.Installed:
                case NewStuff.Entry.Updateable:
                case NewStuff.Entry.Deleted:
                    return "";
                case NewStuff.Entry.Installing:
                    return i18ndc("knewstuff6", "Status message to be shown when the entry is in the process of being installed OR uninstalled", "Currently working on the item %1 by %2. Please wait…", component.name, entryAuthor.name);
                case NewStuff.Entry.Updating:
                    return i18ndc("knewstuff6", "Status message to be shown when the entry is in the process of being updated", "Currently updating the item %1 by %2. Please wait…", component.name, entryAuthor.name);
                default:
                    return i18ndc("knewstuff6", "Status message which should only be shown when the entry has been given some unknown or invalid status.", "This item is currently in an invalid or unknown state. <a href=\"https://bugs.kde.org/enter_bug.cgi?product=frameworks-knewstuff\">Please report this to the KDE Community in a bug report</a>.");
                }
            }

            visible: opacity > 0
            opacity: message.length > 0 ? 1 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    Layout.fillWidth: true
                    text: statusCard.message
                    wrapMode: Text.Wrap
                    onLinkActivated: link => Qt.openUrlExternally(link)
                }

                QQC2.BusyIndicator {
                    running: statusCard.opacity > 0
                }
            }
        }

        Item {
            Layout.fillWidth: true
            height: Kirigami.Units.gridUnit * 3
        }

        Private.EntryScreenshots {
            id: screenshotsItem
            Layout.fillWidth: true
        }

        Kirigami.Heading {
            id: shortSummaryItem
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Kirigami.LinkButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "Comments and Reviews:")
                enabled: component.commentsCount > 0
                text: i18ndc("knewstuff6", "A link which, when clicked, opens a new sub page with comments (comments with or without ratings) for this entry", "%1 Reviews and Comments", component.commentsCount)
                onClicked: mouse => pageStack.push(commentsPage)
            }

            Private.Rating {
                id: ratingsItem
                Kirigami.FormData.label: i18nd("knewstuff6", "Rating:")
                rating: component.rating
            }

            Kirigami.UrlButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "Homepage:")
                text: i18ndc("knewstuff6", "A link which, when clicked, opens the website associated with the entry (this could be either one specific to the project, the author's homepage, or any other website they have chosen for the purpose)", "Open the homepage for %1", component.name)
                url: component.homepage
                visible: url !== ""
            }

            Kirigami.UrlButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "How To Donate:")
                text: i18ndc("knewstuff6", "A link which, when clicked, opens a website with information on donation in support of the entry", "Find out how to donate to this project")
                url: component.donationLink
                visible: url !== ""
            }
        }

        Kirigami.SelectableLabel {
            id: summaryItem

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            textFormat: Text.RichText
        }

        Component {
            id: commentsPage

            Private.EntryCommentsPage {
                itemsModel: component.newStuffModel
                entry: component.entry
                entryName: component.name
                entryAuthorId: component.author.name
                entryProviderId: component.providerId
            }
        }
    }
}
