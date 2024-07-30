/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase

    width: 400
    height: 400
    name: "ScrollablePageTest"
    visible: false

    SignalSpy {
        id: currentItemChangedSpy
        target: mainWindow.pageStack
        signalName: "currentItemChanged"
    }

    // Simulate KCM.ScrollViewKCM
    LingmoUI.ApplicationWindow {
        id: mainWindow
        width: 480
        height: 360
        pageStack.initialPage: LingmoUI.Page {
            id: root
            function clickFirst() {
                userList.itemAtIndex(0).clicked();
            }
            property alias view: scroll.view
            view: ListView {
                id: userList
                model: 1
                delegate: QQC.ItemDelegate {
                    id: delegate
                    width: ListView.view.width
                    text: String(index)
                    onClicked: {
                        mainWindow.pageStack.pop();
                        mainWindow.pageStack.push(subPageComponent);
                        console.log("clicked")
                    }
                }
            }
            QQC.ScrollView {
                id: scroll
                anchors.fill: parent
                property Flickable view
                activeFocusOnTab: false
                contentItem: view
                onViewChanged: {
                    view.parent = scroll;
                }
                LingmoUI.Theme.colorSet: LingmoUI.Theme.View
                LingmoUI.Theme.inherit: false
            }
        }
    }

    Component {
        id: subPageComponent
        LingmoUI.ScrollablePage {
            readonly property alias success: realNametextField.activeFocus
            focus: true
            ColumnLayout {
                QQC.TextField {
                    id: realNametextField
                    focus: true
                    text: "This item should be focused by default"
                }
                QQC.TextField {
                    id: userNameField
                    text: "ddeeff"
                }
            }
        }
    }

    function initTestCase() {
        mainWindow.show()
        mainWindow.requestActivate();
        tryVerify(() => mainWindow.active);
    }

    function cleanupTestCase() {
        mainWindow.close()
    }

    function test_defaultFocusInScrollablePage() {
        mainWindow.pageStack.currentItem.clickFirst();
        if (!(mainWindow.pageStack.currentItem instanceof LingmoUI.ScrollablePage)) {
            currentItemChangedSpy.wait()
        }
        verify(mainWindow.pageStack.currentItem instanceof LingmoUI.ScrollablePage)
        // Consolidate the workaround for QTBUG-44043
        // https://invent.kde.org/frameworks/lingmoui/-/commit/fd253ea5d9fa3f33411e54a596c021f51b5a102a
        tryVerify(() => mainWindow.pageStack.currentItem.success)
    }
}



