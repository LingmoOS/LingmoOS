/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: main

    pageStack.initialPage: LingmoUI.ScrollablePage {
        ListView {
            model: 25
            delegate: LingmoUI.SwipeListItem {
                supportsMouseEvents: false
                actions: [
                    LingmoUI.Action {
                        icon.name: "go-up"
                    }
                ]
                contentItem: Label {
                    elide: Text.ElideRight
                    text: "big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana"
                }
            }
        }
    }
}
