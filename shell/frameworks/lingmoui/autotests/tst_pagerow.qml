/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "GoBack"

    function applicationWindow() { return mainWindow; }

    LingmoUI.ApplicationWindow {
        id: mainWindow
        width: 480
        height: 360
        pageStack.initialPage: LingmoUI.Page {
            Rectangle {
                anchors.fill: parent
                color: "green"
            }
        }
    }

    Component {
        id: randomPage
        LingmoUI.Page {
            Rectangle {
                anchors.fill: parent
                color: "red"
            }
        }
    }

    SignalSpy {
        id: spyCurrentIndex
        target: mainWindow.pageStack
        signalName: "currentIndexChanged"
    }

    SignalSpy {
        id: spyActive
        target: mainWindow
        signalName: "activeChanged"
    }

    function initTestCase() {
        mainWindow.show()
    }

    function cleanupTestCase() {
        mainWindow.close()
    }

    function init() {
        mainWindow.pageStack.clear()
        spyActive.clear()
        spyCurrentIndex.clear()
    }

    function test_pop() {
        compare(mainWindow.pageStack.depth, 0)
        mainWindow.pageStack.push(randomPage)
        compare(mainWindow.pageStack.depth, 1)
        mainWindow.pageStack.pop()
        compare(mainWindow.pageStack.depth, 0)
    }

    function test_goBack() {
        compare(mainWindow.pageStack.depth, 0)
        compare(mainWindow.pageStack.currentIndex, -1)

        let page = mainWindow.pageStack.push(randomPage)
        print(page)
        tryCompare(spyCurrentIndex, "count", 1)
        compare(mainWindow.pageStack.depth, 1)
        compare(mainWindow.pageStack.currentIndex, 0)

        page = mainWindow.pageStack.push(randomPage)
        print(page)
        tryCompare(spyCurrentIndex, "count", 2)
        compare(mainWindow.pageStack.depth, 2)

        compare(mainWindow.pageStack.depth, 2)
        compare(mainWindow.pageStack.currentIndex, 1)

        spyActive.clear()
        mainWindow.requestActivate()
        spyCurrentIndex.clear()
        if (!mainWindow.active)
            spyActive.wait()
        verify(mainWindow.active)
        keyClick(Qt.Key_Left, Qt.AltModifier)

        spyCurrentIndex.wait()

        compare(mainWindow.pageStack.depth, 2)
        compare(mainWindow.pageStack.currentIndex, 0)
        compare(spyCurrentIndex.count, 1)
        mainWindow.pageStack.pop()
        compare(mainWindow.pageStack.depth, 1)
    }

    function test_pushForgettingHistory() {
        let page = mainWindow.pageStack.push(randomPage)
        page.title = "P1"
        tryCompare(spyCurrentIndex, "count", 1)
        page = mainWindow.pageStack.push(randomPage)
        page.title = "P2"
        tryCompare(spyCurrentIndex, "count", 2)
        page = mainWindow.pageStack.push(randomPage)
        page.title = "P3"
        tryCompare(spyCurrentIndex, "count", 3)

        compare(mainWindow.pageStack.depth, 3)
        compare(mainWindow.pageStack.currentIndex, 2)
        mainWindow.pageStack.currentIndex = 0
        page = mainWindow.pageStack.push(randomPage)
        compare(mainWindow.pageStack.depth, 2)
        compare(mainWindow.pageStack.items[0].title, "P1")
        // This is the newly pushed page, everything else went away
        compare(mainWindow.pageStack.items[1].title, "")
        compare(mainWindow.pageStack.items[1], page)
    }

    property int destructions: 0
    Component {
        id: destroyedPage
        LingmoUI.Page {
            id: page
            Rectangle {
                anchors.fill: parent
                color: "blue"
                Component.onDestruction: {
                    testCase.destructions++
                }
            }
        }
    }
    SignalSpy {
        id: spyDestructions
        target: testCase
        signalName: "destructionsChanged"
    }
    function test_clearPages() {
        mainWindow.pageStack.push(destroyedPage)
        mainWindow.pageStack.push(destroyedPage)
        mainWindow.pageStack.push(destroyedPage)
        compare(mainWindow.pageStack.depth, 3)
        mainWindow.pageStack.clear()

        compare(mainWindow.pageStack.depth, 0)
        spyDestructions.wait()
        compare(testCase.destructions, 3)
    }
}
