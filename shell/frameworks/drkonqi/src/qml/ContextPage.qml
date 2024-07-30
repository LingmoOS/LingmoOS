// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

LingmoUI.ScrollablePage {
    id: page

    title: i18nc("@title:window", "What do You Know About the Crash?")

    ColumnLayout {
        LingmoUI.Heading {
            text: xi18nc("@info/rich", "Do you remember what you were doing prior to the crash?")
            level: 2
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        QQC2.ButtonGroup {
            id: rememberGroup
        }
        QQC2.RadioButton {
            text: i18nc("@action:button", "Yes")
            QQC2.ButtonGroup.group: rememberGroup
            onCheckedChanged: reportInterface.userRememberCrashSitutation = checked
        }
        QQC2.RadioButton {
            text: i18nc("@action:button", "No")
            QQC2.ButtonGroup.group: rememberGroup
            checked: true
        }

        LingmoUI.Heading {
            text: xi18nc("@info/rich", "Does the application crash again if you repeat the same situation?")
            level: 2
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        QQC2.ComboBox {
            model: ReproducibilityModel{}
            textRole: "ROLE_String"
            valueRole: "ROLE_Integer"
            onCurrentValueChanged: reportInterface.reproducible = currentValue
        }

        LingmoUI.Heading {
            text: xi18nc("@info/rich", "Please select which additional information you can provide:")
            level: 2
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        QQC2.CheckBox {
            text: xi18nc("@option:check kind of information the user can provide about the crash, %1 is the application name",
              "What I was doing when the application <application>%1</application> crashed",
              DrKonqi.appName())
            onCheckedChanged: reportInterface.provideActionsApplicationDesktop = checked
            Layout.fillWidth: true
        }
        QQC2.CheckBox {
            text: i18nc("@action:check", "Unusual desktop behavior I noticed")
            onCheckedChanged: reportInterface.provideUnusualBehavior = checked
            Layout.fillWidth: true
        }
        QQC2.CheckBox {
            text: i18nc("@action:check", "Custom settings of the application that may be related")
            onCheckedChanged: reportInterface.provideApplicationConfigurationDetails = checked
            Layout.fillWidth: true
        }
    }

    footer: FooterActionBar {
        LingmoUI.PromptDialog {
            id: problemDialog
            title: i18nc("@title", "Not Sufficiently Useful")
            subtitle: xi18nc("@info", "<para>The information you can provide is not considered helpful enough in this case. If you can't think of any more information you can close the bug report dialog.</para>")

            showCloseButton: true
        }

        actions: [
            LingmoUI.Action {
                icon.name: "go-next"
                text: i18nc("@action:button", "Next")
                onTriggered: {
                    if (reportInterface.isBugAwarenessPageDataUseful || DrKonqi.ignoreQuality()) {
                        pageStack.push("qrc:/ui/BacktracePage.qml")
                        return
                    }
                    problemDialog.open()
                }
            }
        ]
    }
}
