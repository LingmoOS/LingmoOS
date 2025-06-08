/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0

import org.kde.private.kscreen 1.0

Item {
    id: root

    // Only show if the user enabled presentation mode or we're a laptop with connected external screens
    Plasmoid.status: presentationModeEnabled || (isLaptop && Plasmoid.nativeInterface.connectedOutputCount > 1) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.toolTipSubText: presentationModeEnabled ? i18n("Presentation mode is enabled") : ""

    readonly property string kcmName: "kcm_kscreen"
    readonly property bool kcmAllowed: KCMShell.authorize(kcmName + ".desktop").length > 0

    readonly property bool presentationModeEnabled: presentationModeCookie > 0
    property int presentationModeCookie: -1

    readonly property bool isLaptop: pmSource.data["PowerDevil"] && pmSource.data["PowerDevil"]["Is Lid Present"]

    function action_configure() {
        KCMShell.openSystemSettings(kcmName);
    }

    PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil", "Inhibitions"]

        onSourceAdded: {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: {
            disconnectSource(source);
        }

        readonly property var inhibitions: {
            var inhibitions = [];

            var data = pmSource.data.Inhibitions;
            if (data) {
                for (var key in data) {
                    if (key === "plasmashell" || key === "plasmoidviewer") { // ignore our own inhibition
                        continue;
                    }

                    inhibitions.push(data[key]);
                }
            }

            return inhibitions;
        }
    }

    // Component.onCompleted: {
    //     if (kcmAllowed) {
    //         Plasmoid.removeAction("configure");
    //         Plasmoid.setAction("configure", i18n("Configure Display Settings…"), "preferences-desktop-display")
    //     }
    // }

    Plasmoid.fullRepresentation: ColumnLayout {
        spacing: 0
        Layout.preferredWidth: PlasmaCore.Units.gridUnit * 15

        ScreenLayoutSelection {
            Layout.leftMargin: PlasmaCore.Units.smallSpacing
            Layout.fillWidth: true
            screenLayouts: Plasmoid.nativeInterface.availableActions().filter(action => action.action !== OsdAction.NoAction)
        }

        PresentationModeItem {
            Layout.fillWidth: true
            Layout.topMargin: PlasmaCore.Units.smallSpacing * 2
            Layout.leftMargin: PlasmaCore.Units.smallSpacing
        }

        // compact the layout
        Item {
            Layout.fillHeight: true
        }
    }
}
