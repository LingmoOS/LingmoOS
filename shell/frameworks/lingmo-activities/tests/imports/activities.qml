/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 2.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.activities 0.1 as Activities

ListView {
    id: main

    property int minimumWidth: 32
    property int minimumHeight: 32 // theme.mSize(theme.defaultFont).height * 14
    property int implicitWidth: minimumWidth * 1.5
    property int implicitHeight: minimumHeight * 1.5

    property int formFactor: plasmoid.formFactor



    model: modelMain

    Activities.ActivityModel {
        id: modelMain
    }

    add: Transition {
        NumberAnimation { properties: "x"; from: 300; duration: 1000 }
    }

    addDisplaced: Transition {
       NumberAnimation { properties: "x,y"; duration: 1000 }
    }

    remove: Transition {
        NumberAnimation { properties: "x"; to: 300; duration: 1000 }
    }

    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 1000 }
    }

    ListModel {
        id: modelDummy

        ListElement {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement {
            name: "Sam Wise"
            number: "555 0473"
        }
    }

    delegate: Column {
        height: 32
        Text {
            text: name
            height: 16
            font.bold: true
        }
        Text {
            text: "   id: " + id
            height: 16
        }

    }
}
