/*
 *   SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
 *   SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

GenericPage
{
    id: container

    required property string path

    // The embedded KCM is expected to bring its own
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    flickable: module.kcm?.mainUi.flickable ?? null
    actions: module.kcm?.mainUi.actions ?? null

    onActiveFocusChanged: {
        if (activeFocus) {
            module.kcm?.mainUi.forceActiveFocus();
        }
    }

    data: [
        Connections {
            target: module.kcm
            function onPagePushed(page) {
                pageStack.layers.push(page);
            }
            function onPageRemoved() {
                pageStack.layers.pop(); //PROBABLY THIS SHOULDN't BE SUPPORTED
            }
            function onNeedsSaveChanged () {
                if (module.kcm.needsSave) {
                    module.kcm.save();
                }
            }
        },
        Connections {
            target: pageStack
            function onPageRemoved(page) {
                if (module.kcm.needsSave) {
                    module.kcm.save();
                }
                if (page == container) {
                    page.destroy();
                }
            }
        },
        Connections {
            target: module.kcm
            function onCurrentIndexChanged(index) {return;
                const index_with_offset = index + 1;
                if (index_with_offset !== pageStack.currentIndex) {
                    pageStack.currentIndex = index_with_offset;
                }
            }
        }
    ]

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent

        width: parent.width - Kirigami.Units.gridUnit * 2

        visible: module.kcm === null

        icon.name: "tools-report-bug"
        text: i18n("This settings module could not be loaded")
        explanation: module.errorString
    }

    Kirigami.Separator {
        width: parent.width
        anchors.bottom: parent.top
    }

    Module {
        id: module
    }

    function load() {
        module.path = container.path;
        module.kcm?.load();
        if (module.kcm !== null) {
            module.kcm.mainUi.parent = contentItem;
            module.kcm.mainUi.anchors.fill = contentItem;
        } else {
            console.warn("Error loading settings module:", path);
            console.warn(" " + module.errorString);
        }
    }

    onPathChanged: load()
}
