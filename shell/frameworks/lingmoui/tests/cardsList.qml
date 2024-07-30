/*
 *  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {

    Component {
        id: delegateComponent
        LingmoUI.Card {
            contentItem: Label { text: ourlist.prefix + index }
        }
    }

    pageStack.initialPage: LingmoUI.ScrollablePage {

        LingmoUI.CardsListView {
            id: ourlist
            property string prefix: "ciao "

            delegate: delegateComponent

            model: 100
        }
    }
}
