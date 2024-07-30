/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff
import org.kde.newstuff.tools.dialog as Myself

Kirigami.ApplicationWindow {
    id: root;
    title: "KNewStuff Dialog"

    globalDrawer: Kirigami.GlobalDrawer {
        id: globalDrawer
        title: "KNewStuff Dialog"
        titleIcon: "get-hot-new-stuff"
        drawerOpen: true
        modal: false

        actions: []
        Instantiator {
            id: configsInstantiator
            model: Myself.KNSRCModel {}
            Kirigami.Action {
                text: model.name
                icon.name: "get-hot-new-stuff"
                onTriggered: {
                    pageStack.clear();
                    pageStack.push(mainPageComponent, { configFile: model.filePath });
                }
            }
            onObjectAdded: (idx, object) => globalDrawer.actions.push(object);
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.defaultColumnWidth: pageStack.width
    Component {
        id: mainPageComponent
        NewStuff.Page { }
    }
    Component {
        id: startPageComponent
        Kirigami.AboutPage {
            aboutData: {
                "displayName": "KNewStuff Dialog",
                "productName": "org.kde.knewstuff.tools.dialog",
                "programLogo": "get-hot-new-stuff",
                "componentName": "knewstuff-dialog",
                "shortDescription": "Get All Your Hot New Stuff",
                "homepage": "https://kde.org/",
                "bugAddress": "https://bugs.kde.org/",
                "version": "v1.0",
                "otherText": "",
                "authors": [
                    {
                        "name": "Dan Leinir Turthra Jensen\n",
                        "task": "Lead Developer",
                        "emailAddress": "admin@leinir.dk",
                        "webAddress": "https://leinir.dk/",
                        "ocsUsername": "leinir",
                    },
                ],
                "credits": [],
                "translators": [],
                "copyrightStatement": "© 2020 The KDE Community",
                "desktopFileName": "org.kde.knewstuff.tools.dialog",
            }
        }
    }
    Component.onCompleted: {
        pageStack.push(startPageComponent);
    }
}
