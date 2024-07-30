// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.Page {
    id: page

    title: i18nc("@title", "Enter the Details about the Crash")

    property bool isComplete: contentCapacityBar.value >= contentCapacityBar.to

    ColumnLayout {
        anchors.fill: parent
        LingmoUI.Heading {
            level: 3
            text: i18nc("@info", "Please provide the following information in English.")
        }

        ColumnLayout {
            RowLayout {
                LingmoUI.Heading {
                    level: 2
                    text: i18nc("@info", "Title of the bug report:")
                }

                LingmoUI.ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip examples of good bug report titles",
`<subtitle>Examples of good titles:</subtitle>
<list>
<item>Lingmo crashed after adding the Notes widget and writing on it</item>
<item>Konqueror crashed when accessing the Facebook application 'X'</item>
<item>Kopete closed after resuming the computer and talking to a MSN buddy</item>
<item>Kate closed while editing a log file and pressing the Delete key a couple of times</item>
</list>`);
                }
            }
            QQC2.TextField {
                id: titleField
                // FIXME: use placeholder text instead of heading??
                Layout.fillWidth: true
                onEditingFinished: reportInterface.title = text
                Accessible.name: i18nc("@info", "Title of the bug report:")
            }
        }

        RowLayout {
            LingmoUI.Heading {
                level: 2
                text: i18nc("@info", "Information about the crash:")
            }

            LingmoUI.ContextualHelpButton {
                toolTipText: xi18nc("@info",
`<subtitle>Describe in as much detail as possible the crash circumstances:</subtitle>
<list>
<item>Detail which actions were you taking inside and outside the application an instant before the crash.</item>
<item>Note if you noticed any unusual behavior in the application or in the whole environment.</item>
<item>Note any non-default configuration in the application</item>
</list>`)
            }

            QQC2.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                wrapMode: Text.Wrap
                text: {
                    if (isComplete) {
                        return i18nc("the minimum required length of a text was reached", "Minimum length reached");
                    }
                    return i18nc("the minimum required length of a text wasn't reached yet", "Provide more information");
                }
            }
            QQC2.ProgressBar {
                id: contentCapacityBar
                from: 0
                to: {
                    const multiplier = (reportInterface.attachToBugNumber == 0) ? 10 : 5
                    return 20 + (reportInterface.selectedOptionsRating() * multiplier)
                }
                value: informationField.text.length
            }
        }
        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            QQC2.TextArea {
                id: informationField
                wrapMode: TextEdit.Wrap
                onEditingFinished: reportInterface.detailText = text
                Accessible.name: i18nc("@info", "Information about the crash:")
            }
        }

        LingmoUI.Heading {
            level: 2
            text: i18nc("@info", "Distribution method:")
        }
        RowLayout {
            QQC2.ComboBox {
                id: platformCombo
                model: PlatformModel {
                    property bool initialized: false
                    manager: bugzilla
                    onDetectedPlatformRowChanged: {
                        if (initialized) {
                            return
                        }
                        initialized = true
                        platformCombo.currentIndex = detectedPlatformRow
                    }
                }
                onCurrentValueChanged: {
                    if (model.initialized) { // lest we screw up initial platform detection
                        DrKonqi.systemInformation.bugzillaPlatform = currentValue
                    }
                 }
            }
            QQC2.CheckBox {
                text: i18nc("@option:check", "KDE Platform is compiled from source")
                onCheckedChanged: DrKonqi.systemInformation.compiledSources = checked
                Component.onCompleted: checked = DrKonqi.systemInformation.compiledSources
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: xi18nc("@info", "<note>The crash and system information will be automatically added to the bug report.</note>")
        }
    }

    footer: FooterActionBar {
        enabled: isComplete
        actions: [
            LingmoUI.Action {
                icon.name: "preview"
                text: i18nc("@action:button", "Preview Report")
                onTriggered: pageStack.push("qrc:/ui/PreviewPage.qml")
            }
        ]
    }

    Component.onCompleted: {
        if (titleField.visible) {
            titleField.forceActiveFocus()
        } else {
            informationField.forceActiveFocus()
        }
    }
}
