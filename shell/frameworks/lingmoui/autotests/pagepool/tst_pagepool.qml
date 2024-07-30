/*
 *  SPDX-FileCopyrightText: 2020 Mason McParlane <mtmcp@outlook.com>
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
    name: "PagePool"

    function initTestCase() {
        mainWindow.show()
    }

    function cleanupTestCase() {
        mainWindow.close()
    }

    function applicationWindow() { return mainWindow; }

    LingmoUI.ApplicationWindow {
        id: mainWindow
        width: 480
        height: 360
    }

    LingmoUI.PagePool {
        id: pool
    }

    function init() {
        mainWindow.pageStack.clear()
        pool.clear()
    }

    // Queries added to page URLs ensure the PagePool can
    // have multiple instances of TestPage.qml

    LingmoUI.PagePoolAction {
        id: loadPageAction
        pagePool: pool
        pageStack: mainWindow.pageStack
        page: "TestPage.qml?action=loadPageAction"
    }

    function test_loadPage () {
        var expectedUrl = "TestPage.qml?action=loadPageAction"
        compare(mainWindow.pageStack.depth, 0)
        loadPageAction.trigger()
        compare(mainWindow.pageStack.depth, 1)
        verify(pool.lastLoadedUrl.toString().endsWith(expectedUrl))
        compare(mainWindow.pageStack.currentItem.title, "INITIAL TITLE")
    }

    LingmoUI.PagePoolAction {
        id: loadPageActionWithProps
        pagePool: pool
        pageStack: mainWindow.pageStack
        page: "TestPage.qml?action=loadPageActionWithProps"
        initialProperties: {
            return {title: "NEW TITLE" }
        }
    }

    function test_loadPageInitialPropertyOverride () {
        var expectedUrl = "TestPage.qml?action=loadPageActionWithProps"
        compare(mainWindow.pageStack.depth, 0)
        loadPageActionWithProps.trigger()
        compare(mainWindow.pageStack.depth, 1)
        verify(pool.lastLoadedUrl.toString().endsWith(expectedUrl))
        compare(mainWindow.pageStack.currentItem.title, "NEW TITLE")
        compare(pool.lastLoadedItem.title, "NEW TITLE")
    }

    LingmoUI.PagePoolAction {
        id: loadPageActionPropsNotObject
        pagePool: pool
        pageStack: mainWindow.pageStack
        page: "TestPage.qml?action=loadPageActionPropsNotObject"
        initialProperties: "This is a string not an object..."
    }

    function test_loadPageInitialPropertiesWrongType () {
        ignoreWarning("initialProperties must be of type object")
        var expectedUrl = "TestPage.qml?action=loadPageAction"
        compare(mainWindow.pageStack.depth, 0)
        loadPageAction.trigger()
        loadPageActionPropsNotObject.trigger()
        compare(mainWindow.pageStack.depth, 1)
        verify(pool.lastLoadedUrl.toString().endsWith(expectedUrl))
    }

    LingmoUI.PagePoolAction {
        id: loadPageActionPropDoesNotExist
        pagePool: pool
        pageStack: mainWindow.pageStack
        page: "TestPage.qml?action=loadPageActionPropDoesNotExist"
        initialProperties: {
            return { propDoesNotExist: "PROP-NON-EXISTENT" }
        }
    }

    function test_loadPageInitialPropertyNotExistFails () {
        ignoreWarning(/.*Setting initial properties failed: TestPage does not have a property called propDoesNotExist/)
        var expectedUrl = "TestPage.qml?action=loadPageActionPropDoesNotExist"
        loadPageActionPropDoesNotExist.trigger()
        verify(pool.lastLoadedUrl.toString().endsWith(expectedUrl))
    }

    function test_contains () {
        const page = "TestPage.qml?action=contains"
        let item = pool.loadPage(page)
        verify(item !== null, "valid item returned from loadPage")
        verify(pool.contains(page), "pool contains page")
        verify(pool.contains(item), "pool contains item")
    }

    function test_deletePageByUrl () {
        const urlPage = "TestPage.qml?action=deletePageByUrl"
        pool.loadPage(urlPage)
        verify(pool.contains(urlPage), "pool contains page before deletion")
        pool.deletePage(urlPage)
        verify(!pool.contains(urlPage), "pool does not contain page after deletion")
    }

    function test_deletePageByItem () {
        const itemPage = "TestPage.qml?action=deletePageByItem"
        let item = pool.loadPage(itemPage)
        verify(pool.contains(item), "pool contains item before deletion")
        pool.deletePage(item)
        verify(!pool.contains(itemPage), "pool does not contain page after deletion")
    }

    function test_iterateAndDeleteByItem () {
        const pages = []
        for (let i = 1; i <= 5; ++i) {
            const page = "TestPage.qml?page=" + i
            pool.loadPage(page)
            verify(pool.contains(page), "pool contains page " + page)
            pages.push(page)
        }
        const items = Array.prototype.slice.call(pool.items);
        compare(items.length, 5, "pool contains 5 items")
        for (const item of items) {
            const url = pool.urlForPage(item)
            const found = pages.find(page => url.toString().endsWith(page))
            verify(found, "pool.items contains page " + found)
            pool.deletePage(item)
        }
        compare(pool.items.length, 0, "all items have been deleted")
    }

    function test_iterateAndDeleteByUrl () {
        const pages = []
        for (let i = 1; i <= 5; ++i) {
            const page = "TestPage.qml?page=" + i
            pool.loadPage(page)
            verify(pool.contains(page), "pool contains page " + page)
            pages.push(page)
        }
        compare(pool.urls.length, 5, "pool contains 5 urls")
        for (const url of pool.urls) {
            const found = pages.find(page => url.toString().endsWith(page))
            verify(found, "pool.urls contains page " + found)
        }
        for (const page of pages) {
            pool.deletePage(page)
        }
        compare(pool.urls.length, 0, "all urls have been deleted")
    }
}
