/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.newstuff as NewStuff

import ".." as Private

Private.GridTileDelegate {
    id: component

    property var entry: model.entry

    function showDetails() {
        pageStack.push(detailsPage, {
            newStuffModel: GridView.view.model,
            entry: model.entry,
        });
    }

    actionsAnchors.topMargin: bigPreview.height + Kirigami.Units.smallSpacing * 2

    actions: [
        Kirigami.Action {
            text: root.useLabel
            icon.name: "dialog-ok-apply"
            onTriggered: source => {
                newStuffModel.engine.adoptEntry(model.entry);
            }
            enabled: (entry.status === NewStuff.Entry.Installed || entry.status === NewStuff.Entry.Updateable) && newStuffEngine.hasAdoptionCommand
            visible: enabled
        },
        Kirigami.Action {
            text: model.downloadLinks.length === 1
                ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install")
                : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…")
            icon.name: "install"
            onTriggered: source => {
                if (model.downloadLinks.length === 1) {
                    newStuffModel.engine.install(model.entry, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = model.downloadLinks;
                    downloadItemsSheet.entry = model.entry;
                    downloadItemsSheet.open();
                }
            }
            enabled: entry.status === NewStuff.Entry.Downloadable || entry.status === NewStuff.Entry.Deleted
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "update-none"
            onTriggered: source => {
                newStuffModel.engine.install(model.entry, NewStuff.ItemsModel.AutoDetectLinkId);
            }
            enabled: entry.status === NewStuff.Entry.Updateable
            visible: enabled
        },
        Kirigami.Action {
            text: root.uninstallLabel
            icon.name: "edit-delete"
            onTriggered: source => {
                newStuffModel.engine.uninstall(model.entry);
            }
            enabled: entry.status === NewStuff.Entry.Installed || entry.status === NewStuff.Entry.Updateable
            visible: enabled
        }
    ]
    thumbnailArea: bigPreview
    thumbnailAvailable: model.previewsSmall.length > 0
    tile: Item {
        anchors {
            fill: parent
            margins: Kirigami.Units.smallSpacing
        }
        ColumnLayout {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: width / 5
                Layout.maximumHeight: width / 1.8
                Kirigami.ShadowedRectangle {
                    visible: bigPreview.status === Image.Ready
                    anchors.centerIn: bigPreview
                    width: Math.min(bigPreview.paintedWidth, bigPreview.width)
                    height: Math.min(bigPreview.paintedHeight, bigPreview.height)
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    shadow.size: 10
                    shadow.color: Qt.rgba(0, 0, 0, 0.3)
                }
                Image {
                    id: bigPreview
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    source: thumbnailAvailable ? model.previews[0] : ""
                    anchors.fill: parent
                }
                Kirigami.Icon {
                    id: updateAvailableBadge
                    opacity: (entry.status === NewStuff.Entry.Updateable) ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                    anchors {
                        top: parent.top
                        left: parent.left
                    }
                    height: Kirigami.Units.iconSizes.medium
                    width: height
                    source: "package-installed-outdated"
                }
                Kirigami.Icon {
                    id: installedBadge
                    opacity: (entry.status === NewStuff.Entry.Installed) ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
                    anchors {
                        top: parent.top
                        left: parent.left
                    }
                    height: Kirigami.Units.iconSizes.medium
                    width: height
                    source: "package-installed-updated"
                }
            }
            Private.Rating {
                Layout.fillWidth: true
                rating: model.rating
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                level: 5
                elide: Text.ElideRight
                text: i18ndc("knewstuff6", "The number of times the item has been downloaded", "%1 downloads", model.downloadCount)
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                elide: Text.ElideRight
                level: 3
                text: model.name
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                elide: Text.ElideRight
                level: 4
                textFormat: Text.StyledText
                text: i18ndc("knewstuff6", "Subheading for the tile view, located immediately underneath the name of the item", "By <i>%1</i>", model.author.name)
            }
            QQC2.Label {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: Kirigami.Units.gridUnit
                Layout.maximumHeight: Kirigami.Units.gridUnit * 3
                wrapMode: Text.Wrap
                text: model.shortSummary.length > 0 ? model.shortSummary : model.summary
                elide: Text.ElideRight
            }
            clip: true // We are dealing with content over which we have very little control. Sometimes that means being a bit abrupt.
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
