/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

Kirigami.ApplicationWindow {
    id: app

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 30
    width: Kirigami.Units.gridUnit * 36
    height: Kirigami.Units.gridUnit * 32

    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.NoNavigationButtons
    pageStack.defaultColumnWidth: width

    footer: Footer {
        width: app.width
        contentSource: {
            switch (Controller.mode) {
                case Controller.Welcome:
                case Controller.Live:
                default:
                    return "FooterDefault.qml";
                case Controller.Update:
                case Controller.Beta:
                    return "FooterUpdate.qml";
            }
        }
    }

    function _createPage(page, isDistroPage = false) {
        let component = Qt.createComponent(page);
        if (component.status !== Component.Error) {
            return component.createObject(null);
        } else {
            let error = component.errorString()
            console.warn("Couldn't load page '" + page + "'");
            console.warn(" " + error);
            component.destroy();

            // Instead return an error page with info
            let errorComponent = Qt.createComponent("PageError.qml");
            if (errorComponent.status !== Component.Error) {
                return errorComponent.createObject(null, {
                    isDistroPage: isDistroPage,
                    error: error
                });
            } else {
                errorComponent.destroy();
                return null;
            }
        }
    }

    function _pushPage(component) {
        if (component !== null) {
            if (pageStack.currentIndex === -1) {
                pageStack.push(component);
            } else {
                // Insert to avoid changing the current page
                pageStack.insertPage(pageStack.depth, component);
            }
        }
    }

    Component.onCompleted: {
        // Push pages dynamically
        switch (Controller.mode) {
            case Controller.Update:
            case Controller.Beta:
                _pushPage(_createPage("PlasmaUpdate.qml"));

                break;

            case Controller.Live:
                _pushPage(_createPage("Live.qml"));
                // Fallthrough

            case Controller.Welcome:
                _pushPage(_createPage("Welcome.qml"));

                if (!Controller.networkAlreadyConnected() || Controller.patchVersion === 80) {
                    _pushPage(_createPage("Network.qml"));
                }

                _pushPage(_createPage("SimpleByDefault.qml"));
                _pushPage(_createPage("PowerfulWhenNeeded.qml"));

                let discover = _createPage("Discover.qml");
                if (discover.application.exists) {
                    _pushPage(discover);
                } else {
                    discover.destroy();
                }

                // KCMs
                if (Controller.mode !== Controller.Live) {
                    if (Controller.userFeedbackAvailable()) {
                        _pushPage(_createPage("Feedback.qml"));
                    }
                }

                // Append any distro-specific pages that were found
                let distroPages = Controller.distroPages();
                for (let i in distroPages) {
                    _pushPage(_createPage(distroPages[i], true));
                }

                _pushPage(_createPage("Enjoy.qml"));

                break;
        }
    }
}
