/*
 *   SPDX-FileCopyrightText: 2013 Antonis Tsiapaliokas <kok3rs@gmail.com>
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

FloatingToolBar {
    id: root

    property Item containment

    signal formFactor(int formFactorType)
    signal location(int locationType)
    signal requestScreenshot()

    function triggerAppletInternalAction(name: string) {
        const applets = containment?.plasmoid.applets;
        if (applets) {
            const applet = applets[0];
            const action = applet?.internalAction(name);
            action?.trigger();
        }
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        PlasmaComponents.Button {
            id: refreshButton
            icon.name: "view-refresh"
            onClicked: root.triggerAppletInternalAction("remove")
        }

        PlasmaComponents.Button {
            id: formFactorMenuButton
            text: i18n("FormFactors")
            onClicked: formFactorMenu.open()
        }

        PlasmaExtras.Menu {
            id: formFactorMenu
            visualParent: formFactorMenuButton
            PlasmaExtras.MenuItem {
                text: i18n("Planar")
                onClicked: root.formFactor(PlasmaCore.Types.Planar)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Vertical")
                onClicked: root.formFactor(PlasmaCore.Types.Vertical)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Horizontal")
                onClicked: root.formFactor(PlasmaCore.Types.Horizontal)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Mediacenter")
                onClicked: root.formFactor(PlasmaCore.Types.MediaCenter)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Application")
                onClicked: root.formFactor(PlasmaCore.Types.Application)
            }
        }

        PlasmaComponents.Button {
            id: locationMenuButton
            text: i18n("Location")
            onClicked: locationMenu.open()
        }

        PlasmaComponents.Button {
            id: screenshotButton
            icon.name: "ksnapshot"
            onClicked: root.requestScreenshot()
        }

        PlasmaExtras.Menu {
            id: locationMenu
            visualParent: locationMenuButton
            PlasmaExtras.MenuItem {
                text: i18n("Floating")
                onClicked: root.location(PlasmaCore.Types.Floating)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Desktop")
                onClicked: root.location(PlasmaCore.Types.Desktop)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Fullscreen")
                onClicked: root.location(PlasmaCore.Types.FullScreen)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Top Edge")
                onClicked: root.location(PlasmaCore.Types.TopEdge)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Bottom Edge")
                onClicked: root.location(PlasmaCore.Types.BottomEdge)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Left Edge")
                onClicked: root.location(PlasmaCore.Types.LeftEdge)
            }
            PlasmaExtras.MenuItem {
                text: i18n("Right Edge")
                onClicked: root.location(PlasmaCore.Types.RightEdge)
            }
        }

        PlasmaComponents.Button {
            id: configButton
            icon.name: "configure"
            onClicked: root.triggerAppletInternalAction("configure")
        }

        PlasmaComponents.Button {
            text: i18n("Configure Containment")
            onClicked: {
                const containment = root.containment?.plasmoid;
                const action = containment?.internalAction("configure");
                action?.trigger();
            }
        }

        PlasmaComponents.Button {
            icon.name: "view-hidden"
            onClicked: {
                root.visible = false;
            }
        }
    }
}
