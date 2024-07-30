/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * A copy of Kirigami.AboutPage adapted to KPluginMetadata instead of KAboutData
 */
ColumnLayout {
    id: root

    property var metaData

    // Icon, name, version, and description
    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            source: root.metaData.iconName
            fallback: "application-x-plasma"
        }

        ColumnLayout {
            Layout.fillWidth: true

            Kirigami.Heading {
                Layout.fillWidth: true
                text: root.metaData.version ? i18ndc("kcmutils6", "Plugin name and plugin version", "%1 %2", root.metaData.name, root.metaData.version) : root.metaData.name
                wrapMode: Text.WordWrap
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                level: 2
                text: root.metaData.description
                wrapMode: Text.WordWrap
            }
        }
    }


    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.bottomMargin: Kirigami.Units.largeSpacing
    }


    // Copyright
    Kirigami.Heading {
        text: i18nd("kcmutils6", "Copyright")
    }
    QQC2.Label {
        Layout.leftMargin: Kirigami.Units.gridUnit
        text: root.metaData.copyrightText
        visible: text.length > 0
    }
    Kirigami.UrlButton {
        Layout.leftMargin: Kirigami.Units.gridUnit
        url: root.metaData.website ? root.metaData.website : ""
        visible: url.length > 0
    }


    // License
    RowLayout {
        QQC2.Label {
            text: i18nd("kcmutils6", "License:")
        }
        Kirigami.LinkButton {
            text: root.metaData.license
            onClicked: {
                licenseSheet.text = root.metaData.licenseText
                licenseSheet.title = root.metaData.license
                licenseSheet.open()
            }
        }
    }


    // Authors, if any
    Item {
        implicitHeight: Kirigami.Units.largeSpacing
        visible: repAuthors.visible
    }
    Kirigami.Heading {
        text: i18nd("kcmutils6", "Authors")
        visible: repAuthors.visible
    }
    Repeater {
        id: repAuthors
        visible: count > 0
        model: root.metaData.authors
        delegate: personDelegate
    }


    // Credits, if any
    Item {
        implicitHeight: Kirigami.Units.largeSpacing
        visible: repCredits.visible
    }
    Kirigami.Heading {
        text: i18nd("kcmutils6", "Credits")
        visible: repCredits.visible
    }
    Repeater {
        id: repCredits
        visible: count > 0
        model: root.metaData.otherContributors
        delegate: personDelegate
    }


    // Translators, if any
    Item {
        implicitHeight: Kirigami.Units.largeSpacing
        visible: repTranslators.visible
    }
    Kirigami.Heading {
        text: i18nd("kcmutils6", "Translators")
        visible: repTranslators.visible
    }
    Repeater {
        id: repTranslators
        visible: count > 0
        model: root.metaData.translators
        delegate: personDelegate
    }


    Component {
        id: personDelegate

        RowLayout {
            height: implicitHeight + (Kirigami.Units.largeSpacing)

            spacing: Kirigami.Units.largeSpacing

            QQC2.Label {
                text: modelData.name
            }
            QQC2.ToolButton {
                visible: modelData.emailAddress
                icon.name: "mail-sent"
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmutils6", "Send an email to %1", modelData.emailAddress)
                onClicked: Qt.openUrlExternally("mailto:%1".arg(modelData.emailAddress))
            }
            QQC2.ToolButton {
                visible: modelData.webAddress
                icon.name: "globe"
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: modelData.webAddress
                onClicked: Qt.openUrlExternally(modelData.webAddress)
            }
        }
    }

    QQC2.Dialog {
        id: licenseSheet
        property alias text: licenseLabel.text

        width: parent.width
        height: parent.height
        anchors.centerIn: parent

        topPadding: 0
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        contentItem: QQC2.ScrollView {
            id: scroll
            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }
            Flickable {
                id: flickable
                contentWidth: width
                contentHeight: licenseLabel.contentHeight
                clip: true
                QQC2.Label {
                    id: licenseLabel
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
