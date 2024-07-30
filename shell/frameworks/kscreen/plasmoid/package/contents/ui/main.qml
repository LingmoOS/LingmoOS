/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.config as KConfig
import org.kde.kcmutils as KCMUtils
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.lingmo5support as P5Support
import org.kde.lingmo.plasmoid
import org.kde.private.kscreen as KScreen

PlasmoidItem {
    id: root

    // Only show if the user enabled presentation mode
    Plasmoid.status: presentationModeEnabled ? LingmoCore.Types.ActiveStatus : LingmoCore.Types.PassiveStatus
    toolTipSubText: presentationModeEnabled ? i18n("Presentation mode is enabled") : ""

    readonly property string kcmName: "kcm_kscreen"
    readonly property bool kcmAllowed: KConfig.KAuthorized.authorizeControlModule("kcm_kscreen")

    readonly property bool presentationModeEnabled: presentationModeCookie > 0
    property int presentationModeCookie: -1

    P5Support.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil", "Inhibitions"]

        onSourceAdded: source => {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: source => {
            disconnectSource(source);
        }

        readonly property var inhibitions: {
            var inhibitions = [];

            var data = pmSource.data.Inhibitions;
            if (data) {
                for (var key in data) {
                    if (key === "lingmoshell" || key === "plasmoidviewer") { // ignore our own inhibition
                        continue;
                    }

                    inhibitions.push(data[key]);
                }
            }

            return inhibitions;
        }
    }

    LingmoCore.Action {
        id: configureAction
        text: i18n("Configure Display Settings…")
        icon.name: "preferences-desktop-display"
        visible: root.kcmAllowed
        onTriggered: KCMUtils.KCMLauncher.openSystemSettings(root.kcmName)
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction);
    }

    fullRepresentation: ColumnLayout {
        spacing: 0
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 15

        ScreenLayoutSelection {
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            Layout.fillWidth: true
            screenLayouts: Plasmoid.availableActions
        }

        PresentationModeItem {
            Layout.fillWidth: true
            Layout.topMargin: LingmoUI.Units.smallSpacing * 2
            Layout.leftMargin: LingmoUI.Units.smallSpacing
        }

        // compact the layout
        Item {
            Layout.fillHeight: true
        }
    }
}
