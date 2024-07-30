// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.19 as Kirigami

import org.kde.drkonqi 1.0

Kirigami.ApplicationWindow {
    id: appWindow

    property var reportInterface: ReportInterface
    property var bugzilla: reportInterface.bugzilla
    property bool canReport: false // can file a bug report
    property bool canAutoReport: false // can file a sentry report
    readonly property bool generatorFailure: BacktraceGenerator.hasAnyFailure

    property string canReportText: {
        canReport = false
        canAutoReport = false

        if (CrashedApplication.bugReportAddress.length <= 0) {
            return xi18nc("@info",
                '<para>You cannot report this error, because <application>%1</application> does not provide a bug reporting address.</para>',
                CrashedApplication.name);
        }

        if (CrashedApplication.fakeExecutableBaseName === "drkonqi") {
            canAutoReport = true
            return xi18nc("@info",
`<para>As the Crash Handler itself has failed, the
automatic reporting process is disabled to reduce the
risks of failing again.<nl /><nl />
Please, <link url='%1'>manually report</link> this error
to the KDE bug tracking system. Do not forget to include
the backtrace from the <interface>Developer Information</interface>
page.</para>`,
                        Globals.ownBugzillaUrl);
        }

        if (DrKonqi.isSafer()) {
            return xi18nc("@info",
`<para>The reporting assistant is disabled because the crash handler dialog was started in safe mode.<nl />
You can manually report this bug to <link>%1</link> (including the backtrace from the <interface>Developer Information</interface> page.)</para>`,
                        CrashedApplication.bugReportAddress);
        }

        if (CrashedApplication.hasDeletedFiles) {
            return xi18nc("@info",
`<para>The reporting assistant is disabled because
the crashed application appears to have been updated or
uninstalled since it had been started. This prevents accurate
crash reporting and can also be the cause of this crash.</para>
<para>After updating it is always a good idea to log out and back
in to make sure the update is fully applied and will not cause
any side effects.</para>`);
        }

        canReport = true
        canAutoReport = true
        return xi18nc("@info",
                        "<para>You can help us improve KDE Software by reporting this error.<nl /><link url='%1'>Learn more about bug reporting.</link></para>",
                        Globals.aboutBugReportingUrl);
    }

    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton | Kirigami.ApplicationHeaderStyle.ShowForwardButton

    title: CrashedApplication.name
    minimumWidth: Kirigami.Settings.isMobile ? 0 : Kirigami.Units.gridUnit * 30
    minimumHeight: Kirigami.Settings.isMobile ? 0 : Kirigami.Units.gridUnit * 22
    height: minimumHeight

    header: generatorFailure ? warningComponent.createObject(appWindow) : null

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    Component {
        id: warningComponent
        Kirigami.InlineMessage {
            text: i18nc("@label", "Gathering crash information failed for unknown reasons. You can retry, or close the window.")
            type: Kirigami.MessageType.Warning
            visible: true
            actions: [
                Kirigami.Action {
                    text: i18nc("@action retry gathering crash data", "Retry")
                    onTriggered: BacktraceGenerator.start()
                }
            ]
        }
    }

    pageStack.initialPage: MainPage {}
    pageStack.defaultColumnWidth: appWindow.width // show single page

    function goToSentry() {
        pageStack.replace("qrc:/ui/SentryPage.qml")
    }
}
