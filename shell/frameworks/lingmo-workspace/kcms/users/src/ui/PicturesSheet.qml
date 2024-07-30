/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 6.3 as Dialogs
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.20 as LingmoUI

LingmoUI.OverlaySheet {
    id: picturesSheet

    title: i18nc("@title", "Change Avatar")
    required property LingmoUI.Page usersDetailPage

    readonly property var colorPalette: [
        {"name": i18nc("@item:intable", "It's Nothing"),     "color": "transparent", "dark": false},
        {"name": i18nc("@item:intable", "Feisty Flamingo"),  "color": "#E93A9A", "dark": true},
        {"name": i18nc("@item:intable", "Dragon's Fruit"),   "color": "#E93D58", "dark": true},
        {"name": i18nc("@item:intable", "Sweet Potato"),     "color": "#E9643A", "dark": true},
        {"name": i18nc("@item:intable", "Ambient Amber"),    "color": "#EF973C", "dark": false},
        {"name": i18nc("@item:intable", "Sparkle Sunbeam"),  "color": "#E8CB2D", "dark": false},
        {"name": i18nc("@item:intable", "Lemon-Lime"),       "color": "#B6E521", "dark": false},
        {"name": i18nc("@item:intable", "Verdant Charm"),    "color": "#3DD425", "dark": false},
        {"name": i18nc("@item:intable", "Mellow Meadow"),    "color": "#00D485", "dark": false},
        {"name": i18nc("@item:intable", "Tepid Teal"),       "color": "#00D3B8", "dark": false},
        {"name": i18nc("@item:intable", "Lingmo Blue"),      "color": "#3DAEE9", "dark": true},
        {"name": i18nc("@item:intable", "Pon Purple"),       "color": "#B875DC", "dark": true},
        {"name": i18nc("@item:intable", "Bajo Purple"),      "color": "#926EE4", "dark": true},
        {"name": i18nc("@item:intable", "Burnt Charcoal"),   "color": "#232629", "dark": true},
        {"name": i18nc("@item:intable", "Paper Perfection"), "color": "#EEF1F5", "dark": false},
        {"name": i18nc("@item:intable", "Cafétera Brown"),   "color": "#CB775A", "dark": false},
        {"name": i18nc("@item:intable", "Rich Hardwood"),    "color": "#6A250E", "dark": true}
    ]

    component PicturesGridLayout: GridLayout {
        rowSpacing: LingmoUI.Units.smallSpacing
        columns: Math.floor((stackSwitcher.implicitWidth - (LingmoUI.Units.gridUnit + LingmoUI.Units.largeSpacing * 2)) / (LingmoUI.Units.gridUnit * 6))
        columnSpacing: LingmoUI.Units.smallSpacing

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Layout.leftMargin: LingmoUI.Units.largeSpacing
        Layout.rightMargin: LingmoUI.Units.largeSpacing
    }

    component PictureButton: QQC2.Button {
        Layout.preferredHeight: LingmoUI.Units.gridUnit * 6
        Layout.preferredWidth: Layout.preferredHeight
        display: QQC2.AbstractButton.IconOnly
    }

    component HomeButton: PictureButton {
        focus: !stackSwitcher.busy
        text: i18nc("@action:button", "Go Back")

        ColumnLayout {
            anchors.centerIn: parent

            LingmoUI.Icon {
                width: LingmoUI.Units.gridUnit * 4
                height: LingmoUI.Units.gridUnit * 4
                source: "go-previous"

                Layout.alignment: Qt.AlignHCenter
            }
        }

        onClicked: stackSwitcher.pop()
    }

    component InitialsButton: PictureButton {
        text: i18nc("@action:button", "Initials")
        property alias colorRectangle: colorRectangle
        property alias color: colorRectangle.color
        property alias headingColor: heading.color

        Rectangle {
            id: colorRectangle

            color: "transparent"
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.smallSpacing

            LingmoUI.Heading {
                id: heading
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing
                font.pixelSize: LingmoUI.Units.gridUnit * 4
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: kcm.initializeString(user.displayPrimaryName)
                textFormat: Text.PlainText
                Accessible.ignored: true
            }
        }
    }

    component IconButton: PictureButton {
        property alias colorRectangle: colorRectangle
        property alias color: colorRectangle.color
        property alias iconColor: icon.color

        Rectangle {
            id: colorRectangle

            color: "transparent"
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.smallSpacing

            LingmoUI.Icon {
                id: icon
                source: "user-identity"
                width: LingmoUI.Units.gridUnit * 4
                height: LingmoUI.Units.gridUnit * 4
                anchors.centerIn: parent
            }
        }
    }

    component MainPage: ColumnLayout {
        PicturesGridLayout {
            PictureButton {
                id: openButton

                text: i18nc("@action:button", "Choose File…")

                contentItem: Item {
                    Dialogs.FileDialog {
                        id: fileDialog
                        title: i18nc("@title", "Choose a picture")
                        onAccepted: {
                            usersDetailPage.oldImage = usersDetailPage.user.face
                            usersDetailPage.user.face = fileDialog.selectedFile
                            usersDetailPage.overrideImage = true
                            picturesSheet.close()
                        }
                    }

                    ColumnLayout {
                        // Centering rather than filling is desired to keep the
                        // entire layout nice and tight when the text is short
                        anchors.centerIn: parent
                        spacing: 0 // the icon should bring its own

                        LingmoUI.Icon {
                            id: openIcon

                            implicitWidth: LingmoUI.Units.iconSizes.huge
                            implicitHeight: LingmoUI.Units.iconSizes.huge
                            source: "document-open"

                            Layout.alignment: Qt.AlignHCenter
                        }
                        QQC2.Label {
                            text: openButton.text
                            textFormat: Text.PlainText

                            Layout.fillWidth: true
                            Layout.maximumWidth: LingmoUI.Units.gridUnit * 5
                            Layout.maximumHeight: openButton.availableHeight - openIcon.height
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignBottom
                            fontSizeMode: Text.HorizontalFit
                            wrapMode: Text.Wrap
                            elide: Text.ElideRight
                        }
                    }
                }

                onClicked: fileDialog.open()
            }

            InitialsButton {
                onClicked: stackSwitcher.push(initialsPage)
            }

            IconButton {
                text: i18nc("@action:button", "Placeholder Icon")
                iconColor: "black"
                onClicked: stackSwitcher.push(iconsPage)
            }

            Repeater {
                model: kcm.avatarFiles
                PictureButton {
                    id: delegate

                    required property string modelData

                    readonly property url source: "file:" + modelData

                    hoverEnabled: true

                    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                    QQC2.ToolTip.text: modelData
                    QQC2.ToolTip.visible: hovered || activeFocus
                    text: modelData

                    LingmoUI.ShadowedImage {
                        id: imgDelegate
                        radius: width
                        anchors.centerIn: parent
                        width: LingmoUI.Units.gridUnit * 5
                        height: LingmoUI.Units.gridUnit * 5
                        sourceSize.width: width
                        sourceSize.height: height
                        asynchronous: true
                        mipmap: true
                        source: delegate.source
                    }

                    onClicked: {
                        usersDetailPage.oldImage = usersDetailPage.user.face
                        usersDetailPage.user.face = delegate.source
                        usersDetailPage.overrideImage = true
                        picturesSheet.close()
                    }
                }
            }
        }
    }

    component InitialsPage: ColumnLayout {
        PicturesGridLayout {
            HomeButton {}

            Repeater {
                model: picturesSheet.colorPalette
                delegate: InitialsButton {
                    color: modelData.color
                    headingColor: modelData.dark ? "white" : "black"
                    hoverEnabled: true

                    text: modelData.name
                    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                    QQC2.ToolTip.text: modelData.name
                    QQC2.ToolTip.visible: hovered || activeFocus

                    onClicked: {
                        colorRectangle.grabToImage(function(result) {
                            const path = kcm.plonkImageInTempfile(result.image)
                            if (path != "") {
                                const uri = "file://" + path
                                usersDetailPage.oldImage = usersDetailPage.user.face
                                usersDetailPage.user.face = uri
                                usersDetailPage.overrideImage = true
                            }
                            picturesSheet.close()
                        })
                    }
                }
            }
        }
    }

    component IconsPage: ColumnLayout {
        PicturesGridLayout {
            HomeButton {}

            Repeater {
                model: picturesSheet.colorPalette
                delegate: IconButton {
                    text: modelData.name
                    color: modelData.color
                    iconColor: modelData.dark ? "white" : "black"

                    Accessible.description: i18nc("@info:whatsthis", "User avatar placeholder icon")

                    onClicked: {
                        colorRectangle.grabToImage(function(result) {
                            const path = kcm.plonkImageInTempfile(result.image)
                            if (path != "") {
                                const uri = "file://" + path
                                usersDetailPage.oldImage = usersDetailPage.user.face
                                usersDetailPage.user.face = uri
                                usersDetailPage.overrideImage = true
                            }
                            picturesSheet.close()
                        })
                    }
                }
            }
        }
    }

    readonly property MainPage mainPage: MainPage {}
    readonly property InitialsPage initialsPage: InitialsPage {}
    readonly property IconsPage iconsPage: IconsPage {}

    onClosed: {
        destroy();
    }

    QQC2.StackView {
        id: stackSwitcher

        implicitWidth: usersDetailPage.width - LingmoUI.Units.largeSpacing * 4
        implicitHeight: currentItem.implicitHeight

        focus: true
        initialItem: mainPage

        Keys.onEscapePressed: picturesSheet.close();
    }

    Component.onCompleted: open()
}
