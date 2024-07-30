/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kcmutils as KCM

/**
 * A copy of LingmoUI.AboutPage adapted to KPluginMetadata instead of KAboutData
 */
KCM.SimpleKCM {
    id: page
    title: i18n("About")

    property var metaData: Plasmoid.metaData

    Component {
        id: personDelegate

        RowLayout {
            height: implicitHeight + (LingmoUI.Units.smallSpacing * 2)

            spacing: LingmoUI.Units.smallSpacing * 2
            LingmoUI.Icon {
                width: LingmoUI.Units.iconSizes.smallMedium
                height: width
                source: "user"
            }
            QQC2.Label {
                text: modelData.name
                textFormat: Text.PlainText
            }
            Row {
                // Group action buttons together
                spacing: 0
                QQC2.ToolButton {
                    visible: modelData.emailAddress
                    width: height
                    icon.name: "mail-sent"

                    display: QQC2.AbstractButton.IconOnly
                    text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Send an email to %1", modelData.emailAddress)

                    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text

                    onClicked: Qt.openUrlExternally("mailto:%1".arg(modelData.emailAddress))
                }
                QQC2.ToolButton {
                    visible: modelData.webAddress
                    width: height
                    icon.name: "globe"

                    display: QQC2.AbstractButton.IconOnly
                    text: i18ndc("lingmo_shell_org.kde.lingmo.desktop", "@info:tooltip %1 url", "Open website %1", modelData.webAddress)

                    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: modelData.webAddress

                    onClicked: Qt.openUrlExternally(modelData.webAddress)
                }
            }
        }
    }

    Component {
        id: licenseComponent

        LingmoUI.OverlaySheet {
            property alias text: licenseLabel.text

            onClosed: destroy()

            LingmoUI.SelectableLabel {
                id: licenseLabel
                implicitWidth: Math.max(LingmoUI.Units.gridUnit * 25, Math.round(page.width / 2), contentWidth)
                wrapMode: Text.WordWrap
            }

            Component.onCompleted: open();
        }
    }

    Item {
        height: childrenRect.height

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: LingmoUI.Units.largeSpacing

            GridLayout {
                columns: 2
                Layout.fillWidth: true

                LingmoUI.Icon {
                    Layout.rowSpan: 2
                    Layout.preferredHeight: LingmoUI.Units.iconSizes.huge
                    Layout.preferredWidth: height
                    Layout.maximumWidth: page.width / 3;
                    Layout.rightMargin: LingmoUI.Units.largeSpacing
                    source: page.metaData.iconName || page.metaData.pluginId
                    fallback: "application-x-lingmo"
                }

                LingmoUI.Heading {
                    Layout.fillWidth: true
                    text: page.metaData.name + " " + page.metaData.version
                    textFormat: Text.PlainText
                }

                LingmoUI.Heading {
                    Layout.fillWidth: true
                    Layout.maximumWidth: LingmoUI.Units.gridUnit * 15
                    level: 2
                    wrapMode: Text.WordWrap
                    text: page.metaData.description
                    textFormat: Text.PlainText
                }
            }

            LingmoUI.Separator {
                Layout.fillWidth: true
            }

            LingmoUI.Heading {
                Layout.topMargin: LingmoUI.Units.smallSpacing
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Copyright")
                textFormat: Text.PlainText
            }

            ColumnLayout {
                spacing: LingmoUI.Units.smallSpacing
                Layout.leftMargin: LingmoUI.Units.smallSpacing

                QQC2.Label {
                    text: page.metaData.copyrightText
                    textFormat: Text.PlainText
                    visible: text.length > 0
                }
                LingmoUI.UrlButton {
                    url: page.metaData.website
                    visible: url.length > 0
                }

                RowLayout {
                    spacing: LingmoUI.Units.smallSpacing
                    QQC2.Label {
                        text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "License:")
                        textFormat: Text.PlainText
                    }
                    LingmoUI.LinkButton {
                        text: page.metaData.license
                        Accessible.description: i18ndc("lingmo_shell_org.kde.lingmo.desktop", "@info:whatsthis", "View license text")
                        onClicked: {
                            licenseComponent.incubateObject(page.Window.window.contentItem, {
                                "text": page.metaData.licenseText,
                                "title": page.metaData.license,
                            }, Qt.Asynchronous);
                        }
                    }
                }
            }

            LingmoUI.Heading {
                Layout.fillWidth: true
                Layout.topMargin: LingmoUI.Units.smallSpacing
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Authors")
                textFormat: Text.PlainText
                visible: page.metaData.authors.length > 0
            }
            Repeater {
                model: page.metaData.authors
                delegate: personDelegate
            }

            LingmoUI.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: LingmoUI.Units.smallSpacing
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Credits")
                textFormat: Text.PlainText
                visible: repCredits.count > 0
            }
            Repeater {
                id: repCredits
                model: page.metaData.otherContributors
                delegate: personDelegate
            }

            LingmoUI.Heading {
                height: visible ? implicitHeight : 0
                Layout.topMargin: LingmoUI.Units.smallSpacing
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Translators")
                textFormat: Text.PlainText
                visible: repTranslators.count > 0
            }
            Repeater {
                id: repTranslators
                model: page.metaData.translators
                delegate: personDelegate
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter

                icon.name: "tools-report-bug"
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Report a Bug…")

                visible: page.metaData.bugReportUrl.length > 0

                onClicked: Qt.openUrlExternally(page.metaData.bugReportUrl)
            }
        }
    }
}
