/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A card based delegate for showing a comment from a KNewStuffQuick::QuickCommentsModel
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.newstuff as NewStuff

RowLayout {
    id: component

    /**
     * The KNSQuick Engine object which handles all our content
     */
    property NewStuff.Engine engine

    /**
     * The username of the author of whatever the comment is attached to
     */
    property string entryAuthorId
    /**
     * The provider ID as supplied by the entry the comment is attached to
     */
    property string entryProviderId

    /**
     * The username of the comment's author
     */
    property string author
    /**
     * The OCS score, an integer from 1 to 100. It will be interpreted
     * as a 5 star rating, with half star support (0-10)
     */
    property int score
    /**
     * The title or subject line for the comment
     */
    property string title
    /**
     * The actual text of the comment
     */
    property alias reviewText: reviewLabel.text
    /**
     * The depth of the comment (in essence, how many parents the comment has)
     */
    property int depth

    spacing: 0

    property NewStuff.Author commentAuthor: NewStuff.Author {
        engine: component.engine
        providerId: component.entryProviderId
        username: component.author
    }

    Repeater {
        model: component.depth
        delegate: Rectangle {
            Layout.fillHeight: true
            Layout.minimumWidth: LingmoUI.Units.largeSpacing
            Layout.maximumWidth: LingmoUI.Units.largeSpacing
            color: Qt.tint(LingmoUI.Theme.textColor, Qt.alpha(LingmoUI.Theme.backgroundColor, 0.8))
            Rectangle {
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                }
                width: 1
                color: LingmoUI.Theme.backgroundColor
            }
        }
    }

    ColumnLayout {
        Item {
            visible: component.depth === 0
            Layout.fillWidth: true
            Layout.minimumHeight: LingmoUI.Units.largeSpacing
            Layout.maximumHeight: LingmoUI.Units.largeSpacing
        }

        LingmoUI.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            visible: (component.title !== "" || component.score !== 0)
            Layout.fillWidth: true
            Layout.leftMargin: LingmoUI.Units.largeSpacing
            LingmoUI.Heading {
                id: titleLabel
                text: ((component.title === "") ? i18ndc("knewstuff6", "Placeholder title for when a comment has no subject, but does have a rating", "<i>(no title)</i>") : component.title)
                level: 4
                Layout.fillWidth: true
            }
            Rating {
                id: ratingStars
                rating: component.score
                reverseLayout: true
            }
            Item {
                Layout.minimumWidth: LingmoUI.Units.largeSpacing
                Layout.maximumWidth: LingmoUI.Units.largeSpacing
            }
        }

        QQC2.Label {
            id: reviewLabel
            Layout.fillWidth: true
            Layout.leftMargin: LingmoUI.Units.largeSpacing
            Layout.rightMargin: LingmoUI.Units.largeSpacing
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            LingmoUI.UrlButton {
                id: authorLabel
                visible: url !== ""
                url: (component.commentAuthor.homepage === "") ? component.commentAuthor.profilepage : component.commentAuthor.homepage
                text: (component.author === component.entryAuthorId)
                    ? i18ndc("knewstuff6", "The author label in case the comment was written by the author of the content entry the comment is attached to", "%1 <i>(author)</i>", component.commentAuthor.name)
                    : component.commentAuthor.name
            }
            QQC2.Label {
                visible: !authorLabel.visible
                text: authorLabel.text
            }
            Image {
                id: authorIcon
                Layout.maximumWidth: height
                Layout.minimumWidth: height
                Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
                fillMode: Image.PreserveAspectFit
                source: component.commentAuthor.avatarUrl
                LingmoUI.Icon {
                    anchors.fill: parent
                    source: "user"
                    visible: opacity > 0
                    opacity: authorIcon.status === Image.Ready ? 0 : 1
                    Behavior on opacity { NumberAnimation { duration: LingmoUI.Units.shortDuration; } }
                }
            }
            Item {
                Layout.minimumWidth: LingmoUI.Units.largeSpacing
                Layout.maximumWidth: LingmoUI.Units.largeSpacing
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: LingmoUI.Units.largeSpacing
            Layout.maximumHeight: LingmoUI.Units.largeSpacing
        }
    }
}
