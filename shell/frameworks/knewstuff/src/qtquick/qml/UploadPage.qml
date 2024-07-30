/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A Kirigami.Page component used for showing how to upload KNS entries to a service
 *
 * This page shows a short guide for uploading new content to the service provided by a KNewStuff
 * provider. This attempts to use the information available through the provider itself, and
 * shows a link to the service's web page, and email in case it is not the KDE Store.
 *
 * While there are not currently any services which support direct OCS API based uploading of
 * new content, we still need a way to guide people to how to do this, hence this component's
 * simplistic nature.
 *
 * This component is functionally equivalent to the old UploadDialog
 * @see KNewStuff::UploadDialog
 * @since 5.85
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

import "private" as Private

Kirigami.ScrollablePage {
    id: component

    /**
     * The NewStuffQuick Engine instance used to display content for this item.
     * You can either pass in one that has already been set up (such as from a
     * NewStuff.Page or NewStuff.Dialog), or you can construct a new one yourself,
     * simply by doing something like this (which will use the wallpapers configuration):
     \code
     NewStuff.UploadPage {
        engine: NewStuff.Engine {
            configFile: "wallpapers.knsrc"
        }
     }
     \endcode
     */
    required property NewStuff.Engine engine

    title: i18nc("@knewstuff6", "Upload New Stuff: %1", engine.name)

    NewStuff.QuestionAsker {
        parent: component.QQC2.Overlay.overlay
    }

    Private.ErrorDisplayer {
        engine: component.engine
        active: component.isCurrentPage
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight

            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            implicitHeight: uploaderBusy.running
                ? uploaderBusy.height + uploaderBusyInfo.height + Kirigami.Units.largeSpacing * 4
                : 0

            visible: uploaderBusy.running
            opacity: uploaderBusy.running ? 1 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            QQC2.BusyIndicator {
                id: uploaderBusy

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.verticalCenter
                    bottomMargin: Kirigami.Units.largeSpacing
                }
                running: component.engine.isLoading && component.engine.isValid
            }

            QQC2.Label {
                id: uploaderBusyInfo

                anchors {
                    top: parent.verticalCenter
                    left: parent.left
                    right: parent.right
                    margins: Kirigami.Units.largeSpacing
                }
                horizontalAlignment: Text.AlignHCenter
                text: i18ndc("knewstuff6", "A text shown beside a busy indicator suggesting that data is being fetched", "Updating information…")
            }
        }

        Repeater {
            model: NewStuff.ProvidersModel {
                engine: component.engine
            }

            Kirigami.Card {
                enabled: !uploaderBusy.running

                banner {
                    title: {
                        if (model.name === "api.kde-look.org") {
                            return i18ndc("knewstuff6", "The name of the KDE Store", "KDE Store");
                        } else if (model.name !== "") {
                            return model.name;
                        } else if (component.engine.name !== "") {
                            return component.engine.name;
                        } else {
                            return i18ndc("knewstuff6", "An unnamed provider", "Your Provider");
                        }
                    }
                    titleIcon: model.icon.toString() === "" ? "get-hot-new-stuff" : model.icon
                }

                actions: [
                    Kirigami.Action {
                        visible: model.website !== ""
                        text: i18ndc("knewstuff6", "Text for an action which causes the specified website to be opened using the user's system default browser", "Open Website: %1", model.website)
                        onTriggered: source => {
                            Qt.openUrlExternally(model.website);
                        }
                    },

                    Kirigami.Action {
                        visible: model.contactEmail !== "" && model.name !== "api.kde-look.org"
                        text: i18ndc("knewstuff6", "Text for an action which will attempt to send an email using the user's system default email client", "Send Email To: %1", model.contactEmail)
                        onTriggered: source => {
                            Qt.openUrlExternally("mailto:" + model.contactEmail);
                        }
                    }
                ]

                contentItem: QQC2.Label {
                    wrapMode: Text.Wrap
                    text: model.name === "api.kde-look.org"
                        ? i18ndc("knewstuff6", "A description of how to upload content to a generic provider", "To upload new entries, or to add content to an existing entry on the KDE Store, please open the website and log in. Once you have done this, you will be able to find the My Products entry in the menu which pops up when you click your user icon. Click on this entry to go to the product management system, where you can work on your products.")
                        : i18ndc("knewstuff6", "A description of how to upload content to the KDE Store specifically", "To upload new entries, or to add content to an existing entry, please open the provider's website and follow the instructions there. You will likely need to create a user and log in to a product management system, where you will need to follow the instructions for how to add. Alternatively, you might be required to contact the managers of the site directly to get new content added.")
                }
            }
        }
    }
}
