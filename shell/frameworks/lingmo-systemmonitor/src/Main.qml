/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtCore
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window

import org.kde.lingmoui as LingmoUI
import org.kde.newstuff as NewStuff

import org.kde.systemmonitor
import org.kde.ksysguard.page as Page

LingmoUI.ApplicationWindow {
    id: app

    minimumWidth: LingmoUI.Units.gridUnit * 34
    minimumHeight: LingmoUI.Units.gridUnit* 27

    title: pageStack.currentItem?.title ?? ""

    header: contentItem.GraphicsInfo.api === GraphicsInfo.Software ? degradedWarning.createObject(app) : null

    Loader {
        active: !LingmoUI.Settings.isMobile
        sourceComponent: GlobalMenu { }
    }

    LingmoUI.PagePool {
        id: pagePoolObject
    }

    globalDrawer: LingmoUI.GlobalDrawer {
        id: globalDrawer

        handleVisible: false
        modal: false
        width: collapsed ? globalToolBar.Layout.minimumWidth + LingmoUI.Units.smallSpacing : Math.max(globalToolBar.implicitWidth + globalToolBar.Layout.minimumWidth + LingmoUI.Units.smallSpacing * 3, LingmoUI.Units.gridUnit * 10)
        Behavior on width { NumberAnimation {id: collapseAnimation; duration: LingmoUI.Units.longDuration; easing.type: Easing.InOutQuad } }
        showHeaderWhenCollapsed: true

        LingmoUI.Theme.colorSet: LingmoUI.Theme.View

        header: ToolBar {
            implicitHeight: app.pageStack.globalToolBar.height

            leftPadding: globalDrawer.collapsed ? 0 : LingmoUI.Units.smallSpacing
            rightPadding: globalDrawer.collapsed ? LingmoUI.Units.smallSpacing / 2 : LingmoUI.Units.smallSpacing
            topPadding: 0
            bottomPadding: 0

            LingmoUI.ActionToolBar {
                id: globalToolBar

                anchors.fill: parent

                overflowIconName: "application-menu"

                actions: [
                    LingmoUI.Action {
                        id: toolsAction
                        text: i18nc("@action", "Tools")
                        icon.name: "tools-symbolic"
                    },
                    LingmoUI.Action {
                        id: editPagesAction
                        icon.name: "handle-sort"
                        text: i18nc("@action", "Edit or Remove pages…")
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onTriggered: pageSortDialog.open()
                    },
                    LingmoUI.Action {
                        id: exportAction
                        text: i18nc("@action", "Export Current Page…")
                        icon.name: "document-export"
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        enabled: !app.pageStack.currentItem?.edit ?? false
                        onTriggered: exportDialog.open()
                    },
                    LingmoUI.Action {
                        icon.name: "document-import"
                        text: i18nc("@action", "Import Page…")
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onTriggered: importDialog.open()
                    },
                    NewStuff.Action {
                        id: ghnsAction
                        text: i18nc("@action:inmenu", "Get New Pages…")
                        configFile: "lingmo-systemmonitor.knsrc"
                        pageStack: app.pageStack.layers
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onEntryEvent: {
                            if (event === NewStuff.Engine.StatusChangedEvent) {
                                pagesModel.ghnsEntryStatusChanged(entry)
                            }
                        }
                    },
                    LingmoUI.Action {
                        id: collapseAction
                        icon.name: app.globalDrawer.collapsed ? "view-split-left-right" : "view-left-close"
                        text: app.globalDrawer.collapsed ? i18nc("@action", "Expand Sidebar") : i18nc("@action", "Collapse Sidebar")
                        onTriggered: app.globalDrawer.collapsed = !app.globalDrawer.collapsed
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                    },
                    LingmoUI.Action {
                        separator: true
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                    },
                    LingmoUI.Action {
                        icon.name: "tools-report-bug";
                        text: i18nc("@action", "Report Bug…");
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onTriggered: Qt.openUrlExternally(CommandLineArguments.aboutData.bugAddress);
                    },
                    LingmoUI.Action {
                        icon.name: "help-about-symbolic";
                        text: i18nc("@action", "About System Monitor");
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onTriggered: app.pageStack.layers.push(Qt.createComponent("org.kde.lingmouiaddons.formcard", "AboutPage"))
                        enabled: app.pageStack.layers.depth <= 1
                    },
                    LingmoUI.Action {
                        icon.name: "kde-symbolic";
                        text: i18nc("@action", "About KDE");
                        displayHint: LingmoUI.DisplayHint.AlwaysHide
                        onTriggered: app.pageStack.layers.push(Qt.createComponent("org.kde.lingmouiaddons.formcard", "AboutKDE"))
                        enabled: app.pageStack.layers.depth <= 1
                    }
                ]
                Component.onCompleted: {
                    app.quitAction.displayHint = LingmoUI.DisplayHint.AlwaysHide
                    actions.push(app.quitAction)
                }
            }

            Instantiator {
                model: ToolsModel { id: toolsModel }

                LingmoUI.Action {
                    text: model.name
                    icon.name: model.icon
                    shortcut: model.shortcut
                    onTriggered: toolsModel.trigger(model.id)
                }

                onObjectAdded: (index, object) => {
                    toolsAction.children.push(object)
                }
            }
        }

        actions: [
            LingmoUI.Action {
                text: i18nc("@action", "Add New Page…")
                icon.name: "list-add"
                onTriggered: pageDialog.open()
            }
        ]

        Instantiator {
            model: Page.PagesModel { id: pagesModel }

            Page.EditablePageAction {
                text: model.title
                icon.name: model.icon
                pagePool: pagePoolObject
                pageData: model.data
                visible: !model.hidden
                onTriggered: {
                    config.lastVisitedPage = model.fileName

                    if (app.pageStack.layers.depth > 0) {
                        app.pageStack.layers.clear()
                    }
                }

                Component.onCompleted: {
                    if (CommandLineArguments.pageId && model.fileName == CommandLineArguments.pageId) {
                        trigger()
                    } else if (CommandLineArguments.pageName && model.title == CommandLineArguments.pageName) {
                        trigger()
                    } else if (config.startPage == model.fileName) {
                        trigger()
                    } else if (config.startPage == "" && config.lastVisitedPage == model.fileName) {
                        trigger()
                    }
                }
            }

            onObjectAdded: (index, object) => {
                var actions = Array.prototype.map.call(globalDrawer.actions, i => i)
                actions.splice(index, 0, object)
                globalDrawer.actions = actions
            }

            onObjectRemoved: (index, object) => {
                var actions = Array.prototype.map.call(globalDrawer.actions, i => i)
                var actionIndex = actions.indexOf(object)
                actions.splice(actionIndex, 1)
                globalDrawer.actions = actions
            }
        }

        contentData: [
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: globalContextMenu.popup(eventPoint.scenePosition.x, eventPoint.scenePosition.y, globalDrawer)
            },
            Menu {
                id: globalContextMenu

                MenuItem { action: editPagesAction }
                MenuItem { action: ghnsAction }
                MenuItem { action: collapseAction }
            }
        ]
    }

    Page.DialogLoader {
        id: pageDialog

        sourceComponent: Page.PageDialog {
            title: i18nc("@window:title", "Add New Page")

            onAccepted: {
                var fileName = name.toLowerCase().replace(" ", "_");
                var newPage = pagesModel.addPage(fileName, {title: name, icon: iconName, margin: margin})
                var row = newPage.insertChild(0, {name: "row-0", isTitle: false, title: "", heightMode: "balanced"})
                var column = row.insertChild(0, {name: "column-0", showBackground: true})
                column.insertChild(0, {name: "section-0", isSeparator: false})
                newPage.savePage()

                const pageAction = Array.from(globalDrawer.actions).find(action => action.pageData.fileName == newPage.fileName)
                pageAction.trigger()
                app.pageStack.currentItem.edit = true
            }
        }
    }

    Page.DialogLoader {
        id: pageSortDialog

        sourceComponent: Page.PageSortDialog {
            title: i18nc("@window:title", "Edit Pages")
            model: pagesModel
            startPage: config.startPage
            onAccepted: {
                config.startPage = startPage
                const currentPage = pageStack.currentItem.pageData.fileName
                const indices = pagesModel.match(pagesModel.index(0, 0), Page.PagesModel.FileNameRole, currentPage, 1,  Qt.MatchExactly)
                if (indices.length == 0 || pagesModel.data(indices[0], Page.PagesModel.HiddenRole)) {
                    if (config.lastVisitedPage == currentPage) {
                        config.lastVisitedPage = "overview.page"
                    }
                    const startPage = config.startPage || config.lastVisitedPage
                    Array.prototype.find.call(globalDrawer.actions, action => action.pageData.fileName == startPage).trigger()
                }
            }
        }
    }

    Page.DialogLoader {
        id: exportDialog
        sourceComponent: FileDialog {
            fileMode: FileDialog.SaveFile
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            title: i18nc("@title:window %1 is the name of the page that is being exported", "Export %1", app.pageStack.currentItem.title)
            defaultSuffix: "page"
            nameFilters: [i18nc("Name filter in file dialog", "System Monitor page (*.page)")]
            onAccepted: app.pageStack.currentItem.pageData.saveAs(selectedFile)
        }
    }
    Page.DialogLoader {
        id: importDialog
        sourceComponent: FileDialog {
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            title: i18nc("@title:window", "Import Page")
            defaultSuffix: "page"
            nameFilters: [i18nc("Name filter in file dialog", "System Monitor page (*.page)")]
            onAccepted: {
                const newPage = pagesModel.importPage(selectedFile)
                if (!newPage) {
                    return;
                }
                const pageAction = Array.from(globalDrawer.actions).find(action => action.pageData.fileName == newPage.fileName)
                pageAction.trigger()
            }
        }
    }

    Configuration {
        id: config
        property alias sidebarCollapsed: globalDrawer.collapsed
        property alias pageOrder: pagesModel.pageOrder
        property alias hiddenPages: pagesModel.hiddenPages
        property string startPage
        property string lastVisitedPage

        property real width
        Binding on width {
            when: app.visibility == Window.Windowed
            delayed: true
            value: app.width
            restoreMode: Binding.RestoreNone
        }

        property real height
        Binding on height {
            when: app.visibility == Window.Windowed
            delayed: true
            value: app.height
            restoreMode: Binding.RestoreNone
        }

        property bool maximized

        Binding on maximized {
            when: app.visibility != Window.Hidden
            // This is delayed otherwise it still writes to the config property even though when is false
            delayed: true
            value: app.visibility == Window.Maximized
            restoreMode: Binding.RestoreNone
        }
    }

    onVisibilityChanged: (visibility) => {
        if (visibility == Window.Windowed) {
            width = config.width
            height = config.height
        }
    }

    Component {
        id: degradedWarning

        ToolBar {
            LingmoUI.InlineMessage {
                anchors.fill: parent
                visible: true
                type: LingmoUI.MessageType.Warning
                text: i18n("System Monitor has fallen back to software rendering because hardware acceleration is not available, and visual glitches may appear. Please check your graphics drivers.")
            }
        }
    }

    pageStack.columnView.columnWidth: LingmoUI.Units.gridUnit * 22
}
