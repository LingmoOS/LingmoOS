/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.newstuff as NewStuff

import ".." as Private

Private.GridTileDelegate {
    id: component

    property var entry: model.entry
    property string useLabel
    property string uninstallLabel

    function showDetails() {
        if (entry.entryType === NewStuff.Entry.GroupEntry) {
            newStuffEngine.storeSearch();
            newStuffEngine.searchTerm = model.payload;
        } else {
            pageStack.push(detailsPage, {
                newStuffModel: GridView.view.model,
                entry,
            });
        }
    }

    actions: [
        LingmoUI.Action {
            text: component.useLabel
            icon.name: "dialog-ok-apply"
            onTriggered: source => {
                newStuffModel.engine.adoptEntry(model.entry);
            }
            enabled: (entry.status === NewStuff.Entry.Installed || entry.status === NewStuff.Entry.Updateable) && newStuffEngine.hasAdoptionCommand
            visible: enabled
        },
        LingmoUI.Action {
            text: model.downloadLinks.length === 1
                ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install")
                : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…")
            icon.name: "install"
            onTriggered: source => {
                if (model.downloadLinks.length === 1) {
                    newStuffEngine.install(entry, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = model.downloadLinks;
                    downloadItemsSheet.entry = entry;
                    downloadItemsSheet.open();
                }
            }
            enabled: entry.status === NewStuff.Entry.Downloadable || entry.status === NewStuff.Entry.Deleted
            visible: enabled
        },
        LingmoUI.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "update-none"
            onTriggered: source => {
                newStuffEngine.install(entry, NewStuff.ItemsModel.AutoDetectLinkId);
            }
            enabled: entry.status === NewStuff.Entry.Updateable
            visible: enabled
        },
        LingmoUI.Action {
            text: component.uninstallLabel
            icon.name: "edit-delete"
            onTriggered: source => {
                newStuffEngine.uninstall(model.entry);
            }
            enabled: entry.status === NewStuff.Entry.Installed || entry.status === NewStuff.Entry.Updateable
            visible: enabled && hovered
        }
    ]
    thumbnailArea: tilePreview
    thumbnailAvailable: model.previewsSmall.length > 0
    tile: Item {
        anchors {
            fill: parent
            margins: LingmoUI.Units.smallSpacing
        }
        GridLayout {
            anchors.fill: parent
            columns: 2
            ColumnLayout {
                Layout.minimumWidth: view.implicitCellWidth / 5
                Layout.maximumWidth: view.implicitCellWidth / 5
                Item {
                    Layout.fillWidth: true
                    Layout.minimumHeight: width
                    Layout.maximumHeight: width
                    LingmoUI.ShadowedRectangle {
                        visible: tilePreview.status === Image.Ready
                        anchors.centerIn: tilePreview
                        width: Math.min(tilePreview.paintedWidth, tilePreview.width)
                        height: Math.min(tilePreview.paintedHeight, tilePreview.height)
                        LingmoUI.Theme.colorSet: LingmoUI.Theme.View
                        shadow.size: LingmoUI.Units.largeSpacing
                        shadow.color: Qt.rgba(0, 0, 0, 0.3)
                    }
                    Image {
                        id: tilePreview
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        source: thumbnailAvailable ? model.previewsSmall[0] : ""
                        anchors {
                            fill: parent
                            margins: LingmoUI.Units.smallSpacing
                        }
                        verticalAlignment: Image.AlignTop
                    }
                    LingmoUI.Icon {
                        id: updateAvailableBadge
                        opacity: (entry.status === NewStuff.Entry.Updateable) ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: LingmoUI.Units.shortDuration } }
                        anchors {
                            top: parent.top
                            left: parent.left
                            margins: -LingmoUI.Units.smallSpacing
                        }
                        height: LingmoUI.Units.iconSizes.smallMedium
                        width: height
                        source: "package-installed-outdated"
                    }
                    LingmoUI.Icon {
                        id: installedBadge
                        opacity: (entry.status === NewStuff.Entry.Installed) ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: LingmoUI.Units.shortDuration } }
                        anchors {
                            top: parent.top
                            left: parent.left
                            margins: -LingmoUI.Units.smallSpacing
                        }
                        height: LingmoUI.Units.iconSizes.smallMedium
                        width: height
                        source: "package-installed-updated"
                    }
                }
                Item {
                    Layout.fillHeight: true
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                LingmoUI.Heading {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    level: 3
                    text: entry.name
                }
                LingmoUI.Heading {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    level: 4
                    textFormat: Text.StyledText
                    text: i18ndc("knewstuff6", "Subheading for the tile view, located immediately underneath the name of the item", "By <i>%1</i>", entry.author.name)
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.Wrap
                    text: entry.shortSummary.length > 0 ? entry.shortSummary : entry.summary
                    elide: Text.ElideRight
                    clip: true // We are dealing with content over which we have very little control. Sometimes that means being a bit abrupt.
                }
            }
            Private.Rating {
                Layout.fillWidth: true
                rating: entry.rating
                visible: entry.entryType === NewStuff.Entry.CatalogEntry
            }
            LingmoUI.Heading {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                level: 5
                elide: Text.ElideRight
                text: i18ndc("knewstuff6", "The number of times the item has been downloaded", "%1 downloads", entry.downloadCount)
                visible: entry.entryType === NewStuff.Entry.CatalogEntry
            }
        }
        FeedbackOverlay {
            anchors.fill: parent
            newStuffModel: component.GridView.view.model
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: mouse => {
                component.showDetails();
            }
        }
    }
}
