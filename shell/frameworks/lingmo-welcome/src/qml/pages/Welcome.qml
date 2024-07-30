/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmouiaddons.formcard as FormCard

import org.kde.lingmo.welcome

GenericPage {
    id: root

    heading: i18nc("@title", "Welcome")
    description: Controller.customIntroText.length > 0
            ? xi18nc("@info:usagetip %1 is custom text supplied by the distro", "%1<nl/><nl/>This operating system is running Lingmo, a free and open-source desktop environment created by KDE, an international software community of volunteers. It is designed to be simple by default for a smooth experience, but powerful when needed to help you really get things done. We hope you love it!", Controller.customIntroText)
            : xi18nc("@info:usagetip %1 is the name of the user's distro", "Welcome to the %1 operating system running KDE Lingmo!<nl/><nl/>Lingmo is a free and open-source desktop environment created by KDE, an international software community of volunteers. It is designed to be simple by default for a smooth experience, but powerful when needed to help you really get things done. We hope you love it!", Controller.distroName())

    actions: [
        LingmoUI.Action {
            text: i18nc("@action:inmenu", "About Welcome Center")
            icon.name: "start-here-kde-lingmo"
            onTriggered: pageStack.layers.push(aboutAppPage)
            displayHint: LingmoUI.DisplayHint.AlwaysHide
        },
        LingmoUI.Action {
            text: i18nc("@action:inmenu", "About KDE")
            icon.name: "kde"
            onTriggered: pageStack.layers.push(aboutKDEPage)
            displayHint: LingmoUI.DisplayHint.AlwaysHide
        }
    ]

    Component {
        id: aboutKDEPage

        FormCard.AboutKDE {}
    }

    Component {
        id: aboutAppPage

        FormCard.AboutPage {
            aboutData: Controller.aboutData
        }
    }

    topContent: [
        LingmoUI.UrlButton {
            id: lingmoLink
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@action:button", "Learn more about the KDE community")
            url: "https://community.kde.org/Welcome_to_KDE?source=lingmo-welcome"
        },
        LingmoUI.UrlButton {
            Layout.topMargin: LingmoUI.Units.largeSpacing
            text: i18nc("@action:button %1 is the name of the user's distro", "Learn more about %1", Controller.distroName())
            url: Controller.distroUrl()
            visible: Controller.distroUrl().length > 0
        }
    ]

    ColumnLayout {
        anchors.centerIn: parent
        height: Math.min(parent.height, LingmoUI.Units.gridUnit * 17)
        spacing: LingmoUI.Units.smallSpacing

        Loader {
            id: imageContainer

            readonly property bool isImage:
                // Image path in the file
                Controller.customIntroIcon.startsWith("file:/") ||
                // Our default image
                Controller.customIntroIcon.length === 0

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.maximumWidth: root.width

            sourceComponent: isImage ? imageComponent : iconComponent

            Component {
                id: imageComponent

                Image {
                    id: image
                    source: Controller.customIntroIcon || "konqi-kde-hi.png"
                    fillMode: Image.PreserveAspectFit

                    LingmoUI.PlaceholderMessage {
                        width: root.width - (LingmoUI.Units.largeSpacing * 4)
                        anchors.centerIn: parent
                        text: i18nc("@title", "Image loading failed")
                        explanation: xi18nc("@info:placeholder", "Could not load <filename>%1</filename>. Make sure it exists.", Controller.customIntroIcon)
                        visible: image.status == Image.Error
                    }
                }
            }

            Component {
                id: iconComponent

                LingmoUI.Icon {
                    implicitWidth: LingmoUI.Units.iconSizes.enormous * 2
                    implicitHeight: implicitWidth
                    source: Controller.customIntroIcon || "kde"
                }
            }

            HoverHandler {
                id: hoverhandler
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                id: tapHandler
                property string url: Controller.customIntroIconLink || lingmoLink.url
                onTapped: Qt.openUrlExternally(url)
            }
            QQC2.ToolTip {
                visible: hoverhandler.hovered
                text: i18nc("@action:button clicking on this takes the user to a web page", "Visit %1", tapHandler.url)
            }
        }

        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: Math.round(Math.max(root.width / 2, imageContainer.implicitWidth / 2))
            text: Controller.customIntroIconCaption || i18nc("@info", "The KDE mascot Konqi welcomes you to the KDE community!")
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
