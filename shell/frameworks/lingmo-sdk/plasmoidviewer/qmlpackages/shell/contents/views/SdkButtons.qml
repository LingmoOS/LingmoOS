/*
 *   SPDX-FileCopyrightText: 2013 Antonis Tsiapaliokas <kok3rs@gmail.com>
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras

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
        spacing: LingmoUI.Units.smallSpacing

        LingmoComponents.Button {
            id: refreshButton
            icon.name: "view-refresh"
            onClicked: root.triggerAppletInternalAction("remove")
        }

        LingmoComponents.Button {
            id: formFactorMenuButton
            text: i18n("FormFactors")
            onClicked: formFactorMenu.open()
        }

        LingmoExtras.Menu {
            id: formFactorMenu
            visualParent: formFactorMenuButton
            LingmoExtras.MenuItem {
                text: i18n("Planar")
                onClicked: root.formFactor(LingmoCore.Types.Planar)
            }
            LingmoExtras.MenuItem {
                text: i18n("Vertical")
                onClicked: root.formFactor(LingmoCore.Types.Vertical)
            }
            LingmoExtras.MenuItem {
                text: i18n("Horizontal")
                onClicked: root.formFactor(LingmoCore.Types.Horizontal)
            }
            LingmoExtras.MenuItem {
                text: i18n("Mediacenter")
                onClicked: root.formFactor(LingmoCore.Types.MediaCenter)
            }
            LingmoExtras.MenuItem {
                text: i18n("Application")
                onClicked: root.formFactor(LingmoCore.Types.Application)
            }
        }

        LingmoComponents.Button {
            id: locationMenuButton
            text: i18n("Location")
            onClicked: locationMenu.open()
        }

        LingmoComponents.Button {
            id: screenshotButton
            icon.name: "ksnapshot"
            onClicked: root.requestScreenshot()
        }

        LingmoExtras.Menu {
            id: locationMenu
            visualParent: locationMenuButton
            LingmoExtras.MenuItem {
                text: i18n("Floating")
                onClicked: root.location(LingmoCore.Types.Floating)
            }
            LingmoExtras.MenuItem {
                text: i18n("Desktop")
                onClicked: root.location(LingmoCore.Types.Desktop)
            }
            LingmoExtras.MenuItem {
                text: i18n("Fullscreen")
                onClicked: root.location(LingmoCore.Types.FullScreen)
            }
            LingmoExtras.MenuItem {
                text: i18n("Top Edge")
                onClicked: root.location(LingmoCore.Types.TopEdge)
            }
            LingmoExtras.MenuItem {
                text: i18n("Bottom Edge")
                onClicked: root.location(LingmoCore.Types.BottomEdge)
            }
            LingmoExtras.MenuItem {
                text: i18n("Left Edge")
                onClicked: root.location(LingmoCore.Types.LeftEdge)
            }
            LingmoExtras.MenuItem {
                text: i18n("Right Edge")
                onClicked: root.location(LingmoCore.Types.RightEdge)
            }
        }

        LingmoComponents.Button {
            id: configButton
            icon.name: "configure"
            onClicked: root.triggerAppletInternalAction("configure")
        }

        LingmoComponents.Button {
            text: i18n("Configure Containment")
            onClicked: {
                const containment = root.containment?.plasmoid;
                const action = containment?.internalAction("configure");
                action?.trigger();
            }
        }

        LingmoComponents.Button {
            icon.name: "view-hidden"
            onClicked: {
                root.visible = false;
            }
        }
    }
}
