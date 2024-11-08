/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "fm-window.h"

#include "tab-page.h"
#include "side-bar.h"
#include "navigation-bar.h"
#include "tool-bar.h"
#include "search-bar.h"
#include "status-bar.h"
#include "path-completer.h"
#include "path-edit.h"

#include "search-vfs-uri-parser.h"

#include "directory-view-container.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-widget.h"

#include "directory-view-menu.h"

#include "file-utils.h"
#include "file-operation-utils.h"
#include "file-info.h"

#include "file-operation-manager.h"

#include "preview-page-plugin-iface.h"
#include "preview-page-factory-manager.h"

#include "directory-view-factory-manager.h"

//#include "properties-window.h"
#include "properties-window-factory-plugin-manager.h"

#include "global-settings.h"

#include <QDesktopServices>
#include <QUrl>

#include <QDockWidget>
#include <QStandardPaths>
#include <QDebug>

#include <QSplitter>
#include <QHBoxLayout>
#include <QPainter>

#include <QMessageBox>
#include <QLabel>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QStringListModel>
#include <QFileDialog>

#include <QLayout>

#include <QKeyEvent>

#include <QApplication>

using namespace Peony;

FMWindow::FMWindow(const QString &uri, QWidget *parent) : QMainWindow (parent)
{
    auto settings = GlobalSettings::getInstance();
    m_show_hidden_file = settings->isExist(SHOW_HIDDEN_PREFERENCE)? settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
    m_use_default_name_sort_order = settings->isExist(SORT_CHINESE_FIRST)? settings->getValue(SORT_CHINESE_FIRST).toBool(): false;
    m_folder_first = settings->isExist(SORT_FOLDER_FIRST)? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;

    setWindowIcon(QIcon::fromTheme("system-file-manager"));
    setWindowTitle(tr("File Manager"));

    m_operation_minimum_interval.setSingleShot(true);

    connect(qApp, &QApplication::paletteChanged, this, [=]() {
        this->repaint();
        for (auto child : this->children()) {
            QWidget *widget = qobject_cast<QWidget*>(child);
            if (widget) {
                widget->repaint();
            }
        }
    });

    //setStyleSheet(".Peony--FMWindow::separator{border:0; padding:0}");
    setAttribute(Qt::WA_DeleteOnClose);
    //setAnimated(false);
    /*
    QTimer::singleShot(1000, [=](){
        QDockWidget *d = new QDockWidget(this);
        //d->setFeatures(QDockWidget::DockWidgetClosable);
        auto l = new QLabel(tr("test"), d);
        d->setTitleBarWidget(l);
        //d->setWidget(new QLabel(tr("test"), d));
        addDockWidget(Qt::TopDockWidgetArea, d);
        QTimer::singleShot(1000, [=](){
            d->hide();
            QTimer::singleShot(1000, [=](){
                d->show();
            });
        });
    });
     */

    auto location = uri;
    if (uri.isNull()) {
        location = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    //init to solve second search in empty path issue
    m_last_non_search_location = location;

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setLayoutDirection(Qt::LeftToRight);
    m_splitter->setHandleWidth(-0.5);
    m_splitter->setStyleSheet("QSplitter"
                              "{"
                              "border: 0;"
                              "padding: 0;"
                              "margin: 0;"
                              "}");

    setCentralWidget(m_splitter);

    m_tab = new TabPage(this);
    m_tab->addPage(location);

    m_side_bar = new SideBar(this);
    m_filter_bar = new AdvanceSearchBar(this);
    m_filter_bar->setMinimumWidth(180);
    m_filter = dynamic_cast<QWidget*>(m_filter_bar);

    m_side_bar_container = new QStackedWidget(this);
    m_side_bar_container->addWidget(m_side_bar);
    m_side_bar_container->addWidget(m_filter);
    m_side_bar_container->setCurrentWidget(m_side_bar);
    m_splitter->addWidget(m_side_bar_container);

    m_splitter->addWidget(m_tab);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setStretchFactor(2, 1);

    m_tool_bar = new ToolBar(this, this);
    m_tool_bar->setContentsMargins(0, 0, 0, 0);

    m_search_bar = new SearchBar(this);
    m_search_bar->setMinimumWidth(200);
    m_advanced_button = new QPushButton(tr("advanced search"), nullptr);
    m_advanced_button->setFixedWidth(110);
    m_advanced_button->setStyleSheet("color: rgb(10,10,255)");
    m_clear_record = new QPushButton(tr("clear record"), nullptr);
    m_clear_record->setFixedWidth(110);
    m_clear_record->setDisabled(true);
    //set button hidden, function not open to public yet
    m_advanced_button->setVisible(false);
    m_clear_record->setVisible(false);

    m_preview_page_container = new PreviewPageContainer(this);
    m_splitter->addWidget(m_preview_page_container);
    m_preview_page_container->hide();

    //put the tool bar and search bar into
    //a hobx-layout widget, and put the widget int a
    //new tool bar.
    QWidget *w1 = new QWidget(this);
    w1->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *l1 = new QHBoxLayout(w1);
    l1->setContentsMargins(5, 0, 0, 0);
    w1->setLayout(l1);
    l1->addWidget(m_tool_bar, Qt::AlignLeft);
    l1->addWidget(m_search_bar, Qt::AlignRight);
    l1->addWidget(m_advanced_button, Qt::AlignRight);
    l1->addWidget(m_clear_record, Qt::AlignRight);

    ToolBarContainer *t1 = new ToolBarContainer(this);
    t1->setContentsMargins(0, 0, 10, 0);
    t1->setMovable(false);
    t1->addWidget(w1);

    addToolBar(Qt::TopToolBarArea, t1);
    addToolBarBreak();

    m_navigation_bar = new NavigationBar(this);
    m_navigation_bar->setMovable(false);
    m_navigation_bar->bindContainer(m_tab->getActivePage());
    m_navigation_bar->updateLocation(location);

    QToolBar *t = new QToolBar(this);
    t->setStyleSheet(".QToolBar{border: 0; padding: 0}");
    t->setMovable(false);
    t->addWidget(m_navigation_bar);
    t->setContentsMargins(0, 0, 0, 0);
    addToolBar(t);

    m_status_bar = new StatusBar(this, this);
    setStatusBar(m_status_bar);

    m_navigation_bar->bindContainer(m_tab->getActivePage());

    //connect signals
    connect(m_side_bar, &SideBar::updateWindowLocationRequest, this, &FMWindow::goToUri);
    connect(m_tab, &TabPage::updateWindowLocationRequest, this, &FMWindow::goToUri);
    connect(m_navigation_bar, &NavigationBar::updateWindowLocationRequest, this, &FMWindow::goToUri);
    connect(m_navigation_bar, &NavigationBar::refreshRequest, this, &FMWindow::refresh);
    connect(m_advanced_button, &QPushButton::clicked, this, &FMWindow::advanceSearch);
    connect(m_clear_record, &QPushButton::clicked, this, &FMWindow::clearRecord);

    //tab changed
    connect(m_tab, &TabPage::currentActiveViewChanged, [=]() {
        this->m_tool_bar->updateLocation(getCurrentUri());
        this->m_tool_bar->updateStates();
        this->m_navigation_bar->bindContainer(getCurrentPage());
        this->m_navigation_bar->updateLocation(getCurrentUri());
        this->m_status_bar->update();
        Q_EMIT this->tabPageChanged();
        if (m_filter_visible)
        {
            advanceSearch();
            filterUpdate();
        }
    });

    //location change
    connect(m_tab, &TabPage::currentLocationChanged,
            this, &FMWindow::locationChangeEnd);
    connect(this, &FMWindow::locationChangeStart, [=]() {
        m_is_loading = true;
        m_side_bar->blockSignals(true);
        m_tool_bar->blockSignals(true);
        m_navigation_bar->setBlock(true);
    });
    connect(this, &FMWindow::locationChangeEnd, [=]() {
        //qDebug()<<this->getCurrentAllFileUris();
        m_is_loading = false;
        m_side_bar->blockSignals(false);
        m_tool_bar->blockSignals(false);
        m_navigation_bar->setBlock(false);
        qDebug()<<this->getCurrentUri();
        m_navigation_bar->updateLocation(getCurrentUri());
        m_tool_bar->updateLocation(getCurrentUri());
        m_tool_bar->updateStates();
    });

    //selection changed
    connect(m_tab, &TabPage::currentSelectionChanged, [=]() {
        m_status_bar->update();
        m_tool_bar->updateStates();
        Q_EMIT this->windowSelectionChanged();
    });

    //location change
    connect(this, &FMWindow::locationChangeStart, [this]() {
        QCursor c;
        c.setShape(Qt::WaitCursor);
        this->setCursor(c);
        m_status_bar->update(tr("Loaing... Press Esc to stop a loading."));
    });

    connect(this, &FMWindow::locationChangeEnd, [this]() {
        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        m_status_bar->update();
    });

    //view switched
    connect(m_tab, &TabPage::viewTypeChanged, [=]() {
        m_tool_bar->updateLocation(getCurrentUri());
        m_tool_bar->updateStates();
    });

    //search, comment old code
//    connect(m_search_bar, &SearchBar::searchKeyChanged, [=]() {
//        //FIXME: filter the current directory
//    });
//    connect(m_search_bar, &SearchBar::searchRequest, [=](const QString &key) {
//        QString uri = this->getCurrentUri();
//        if (uri.startsWith("search:///")) {
//            uri = m_last_non_search_location;
//        }
//        m_update_condition = false; //common search, no filter
//        auto targetUri = SearchVFSUriParser::parseSearchKey(uri, key);
//        this->goToUri(targetUri, true);
//        m_clear_record->setDisabled(false); //has record to clear
//    });

    //action
    QAction *stopLoadingAction = new QAction(this);
    stopLoadingAction->setShortcut(QKeySequence(Qt::Key_Escape));
    addAction(stopLoadingAction);
    connect(stopLoadingAction, &QAction::triggered, this, &FMWindow::forceStopLoading);

    //show hidden action
    QAction *showHiddenAction = new QAction(this);
    showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    addAction(showHiddenAction);
    connect(showHiddenAction, &QAction::triggered, this, [=](bool checked) {
        this->setShowHidden(checked);
    });

    auto undoAction = new QAction(QIcon::fromTheme("edit-undo-symbolic"), tr("Undo"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    addAction(undoAction);
    connect(undoAction, &QAction::triggered, [=]() {
        FileOperationManager::getInstance()->undo();
    });

    auto redoAction = new QAction(QIcon::fromTheme("edit-redo-symbolic"), tr("Redo"), this);
    redoAction->setShortcut(QKeySequence::Redo);
    addAction(redoAction);
    connect(redoAction, &QAction::triggered, [=]() {
        FileOperationManager::getInstance()->redo();
    });

    auto trashAction = new QAction(this);
    trashAction->setShortcut(Qt::Key_Delete);
    connect(trashAction, &QAction::triggered, [=]() {
        auto uris = this->getCurrentSelections();
        if (!uris.isEmpty()) {
            bool isTrash = this->getCurrentUri() == "trash:///";
            if (!isTrash) {
                FileOperationUtils::trash(uris, true);
            } else {
                FileOperationUtils::executeRemoveActionWithDialog(uris);
            }
        }
    });
    addAction(trashAction);

    auto deleteAction = new QAction(this);
    deleteAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    addAction(deleteAction);
    connect(deleteAction, &QAction::triggered, [=]() {
        auto uris = this->getCurrentSelections();
        FileOperationUtils::executeRemoveActionWithDialog(uris);
    });

    auto searchAction = new QAction(this);
    searchAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::Key_F)<<QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(searchAction, &QAction::triggered, this, [=]() {
        m_search_bar->setFocus();
    });
    addAction(searchAction);

    auto locationAction = new QAction(this);
    locationAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F4<<QKeySequence(Qt::ALT + Qt::Key_D));
    connect(locationAction, &QAction::triggered, this, [=]() {
        m_navigation_bar->startEdit();
    });
    addAction(locationAction);

    auto newWindowAction = new QAction(this);
    newWindowAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    connect(newWindowAction, &QAction::triggered, this, [=]() {
        FMWindow *newWindow = new FMWindow(getCurrentUri());
        newWindow->show();
    });
    addAction(newWindowAction);

    auto closeWindowAction = new QAction(this);
    closeWindowAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_F4));
    connect(closeWindowAction, &QAction::triggered, this, [=]() {
        this->close();
    });
    addAction(closeWindowAction);

    auto aboutAction = new QAction(this);
    aboutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F2));
    connect(aboutAction, &QAction::triggered, this, [=]() {
        QMessageBox::about(this,
                           tr("Peony Qt"),
                           tr("Author:\n"
                              "\tYue Lan <lanyue@kylinos.cn>\n"
                              "\tMeihong He <hemeihong@kylinos.cn>\n"
                              "\n"
                              "Copyright (C): 2019-2020, Tianjin LINGMO Information Technology Co., Ltd."));
    });
    addAction(aboutAction);

    auto newTabAction = new QAction(this);
    newTabAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connect(newTabAction, &QAction::triggered, this, [=]() {
        this->addNewTabs(QStringList()<<this->getCurrentUri());
    });
    addAction(newTabAction);

    auto closeTabAction = new QAction(this);
    closeTabAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
    connect(closeTabAction, &QAction::triggered, this, [=]() {
        if (m_tab->count() <= 1) {
            this->close();
        } else {
            m_tab->removeTab(m_tab->currentIndex());
        }
    });
    addAction(closeTabAction);

    auto nextTabAction = new QAction(this);
    nextTabAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab));
    connect(nextTabAction, &QAction::triggered, this, [=]() {
        int currentIndex = m_tab->currentIndex();
        if (currentIndex + 1 < m_tab->count()) {
            m_tab->setCurrentIndex(currentIndex + 1);
        } else {
            m_tab->setCurrentIndex(0);
        }
    });
    addAction(nextTabAction);

    auto previousTabAction = new QAction(this);
    previousTabAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab));
    connect(previousTabAction, &QAction::triggered, this, [=]() {
        int currentIndex = m_tab->currentIndex();
        if (currentIndex > 0) {
            m_tab->setCurrentIndex(currentIndex - 1);
        } else {
            m_tab->setCurrentIndex(m_tab->count() - 1);
        }
    });
    addAction(previousTabAction);

    auto newFolderAction = new QAction(this);
    newFolderAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(newFolderAction, &QAction::triggered, this, [=]() {
        CreateTemplateOperation op(getCurrentUri(), CreateTemplateOperation::EmptyFolder, tr("New Folder"));
        op.run();
        auto targetUri = op.target();

        QTimer::singleShot(500, this, [=]() {
            this->getCurrentPage()->getView()->scrollToSelection(targetUri);
            this->editUri(targetUri);
        });
    });
    addAction(newFolderAction);

    auto propertiesWindowAction = new QAction(this);
    propertiesWindowAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    connect(propertiesWindowAction, &QAction::triggered, this, [=]() {
        if (getCurrentSelections().count() >0)
        {
            QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(getCurrentSelections());
            //PropertiesWindow *w = new PropertiesWindow(getCurrentSelections());
            w->show();
        }
    });
    addAction(propertiesWindowAction);

    auto helpAction = new QAction(this);
    helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    connect(helpAction, &QAction::triggered, this, [=]() {
        QUrl url = QUrl("help:ubuntu-lingmo-help/files", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
    });
    addAction(helpAction);

    auto maxAction = new QAction(this);
    maxAction->setShortcut(QKeySequence(Qt::Key_F11));
    connect(maxAction, &QAction::triggered, this, [=]() {
        if (!this->isFullScreen()) {
            this->showFullScreen();
        } else {
            this->showMaximized();
        }
    });
    addAction(maxAction);

    auto previewPageAction = new QAction(this);
    previewPageAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F3<<QKeySequence(Qt::ALT + Qt::Key_P));
    connect(previewPageAction, &QAction::triggered, this, [=]() {
        auto lastPreviewPageId = m_navigation_bar->getLastPreviewPageId();
        m_navigation_bar->triggerAction(lastPreviewPageId);
    });
    addAction(previewPageAction);

    auto refreshAction = new QAction(this);
    refreshAction->setShortcut(Qt::Key_F5);
    connect(refreshAction, &QAction::triggered, this, [=]() {
        this->refresh();
    });
    addAction(refreshAction);

    //menu
    m_tab->connect(m_tab, &TabPage::menuRequest, [=]() {
        if (m_is_loading)
            return;
        DirectoryViewMenu menu(this, nullptr);
        menu.exec(QCursor::pos());
        auto urisToEdit = menu.urisToEdit();
        if (!urisToEdit.isEmpty())
        {
            QTimer::singleShot(100, this, [=]() {
                this->getCurrentPage()->getView()->scrollToSelection(urisToEdit.first());
                this->editUri(urisToEdit.first());
            });
        }
    });

    //preview page
    connect(m_navigation_bar, &NavigationBar::switchPreviewPageRequest,
            this, &FMWindow::onPreviewPageSwitch);
    connect(m_tab, &TabPage::currentSelectionChanged, [=]() {
        if (m_preview_page_container->getCurrentPage()) {
            auto selection = getCurrentSelections();
            if (!selection.isEmpty()) {
                m_preview_page_container->getCurrentPage()->cancel();
                m_preview_page_container->getCurrentPage()->prepare(selection.first());
                m_preview_page_container->getCurrentPage()->startPreview();
            } else {
                m_preview_page_container->getCurrentPage()->cancel();
            }
        }
    });
    connect(m_tab, &TabPage::currentLocationChanged, [=]() {
        if (m_preview_page_container->getCurrentPage()) {
            m_preview_page_container->getCurrentPage()->cancel();
        }
    });
    connect(m_tab, &TabPage::currentActiveViewChanged, [=]() {
        if (m_preview_page_container->getCurrentPage()) {
            auto selection = getCurrentSelections();
            if (selection.isEmpty()) {
                m_preview_page_container->getCurrentPage()->cancel();
            } else {
                m_preview_page_container->getCurrentPage()->prepare(selection.first());
                m_preview_page_container->getCurrentPage()->startPreview();
            }
        }
    });
}

FMWindowFactory *FMWindow::getFactory()
{
    return nullptr;
}

FMWindowIface *FMWindow::create(const QString &uri)
{
    return new FMWindow(uri);
}

FMWindowIface *FMWindow::create(const QStringList &uris)
{
    if (uris.isEmpty())
        return nullptr;
    auto window = new FMWindow(uris.first());
    QStringList l;
    for (auto uri : uris) {
        if (uris.indexOf(uri) != 0) {
            l<<uri;
        }
    }
    if (!l.isEmpty())
        window->addNewTabs(l);
    return window;
}

const QString FMWindow::getCurrentUri()
{
    if (m_tab->getActivePage()) {
        return m_tab->getActivePage()->getCurrentUri();
    }
    return nullptr;
}

const QStringList FMWindow::getCurrentAllFileUris()
{
    if (m_tab->getActivePage()) {
        return m_tab->getActivePage()->getAllFileUris();
    }
    return QStringList();
}

const QStringList FMWindow::getCurrentSelections()
{
    if (m_tab->getActivePage()) {
        return m_tab->getActivePage()->getCurrentSelections();
    }
    return QStringList();
}

const QList<std::shared_ptr<FileInfo>> FMWindow::getCurrentSelectionFileInfos()
{
    const QStringList uris = getCurrentSelections();
    QList<std::shared_ptr<FileInfo>> infos;
    for(auto uri : uris) {
        auto info = FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}

void FMWindow::addNewTabs(const QStringList &uris)
{
    for (auto uri : uris) {
        m_tab->addPage(uri);
    }
}

void FMWindow::goToUri(const QString &uri, bool addHistory, bool forceUpdate)
{
    QUrl url(uri);
    auto realUri = uri;
    if (url.scheme().isEmpty()) {
        if (uri.startsWith("/")) {
            realUri = "file://" + uri;
        } else {
            QDir currentDir = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
            currentDir.cd(uri);
            auto absPath = currentDir.absoluteFilePath(uri);
            url = QUrl::fromLocalFile(absPath);

            realUri = url.toDisplayString();
        }
    }

    if (forceUpdate)
        goto update;

    if (getCurrentUri() == realUri)
        return;

update:
    if (!realUri.startsWith("search://")) {
        m_last_non_search_location = realUri;
    }
    Q_EMIT locationChangeStart();
    if (m_update_condition)
        filterUpdate();
    m_tab->getActivePage()->goToUri(realUri, addHistory, forceUpdate);
    m_tab->refreshCurrentTabText();
    m_navigation_bar->updateLocation(realUri);
    m_tool_bar->updateLocation(realUri);
}

void FMWindow::beginSwitchView(const QString &viewId)
{
    m_tab->getActivePage()->switchViewType(viewId);
}

void FMWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
}

DirectoryViewContainer *FMWindow::getCurrentPage()
{
    return m_tab->getActivePage();
}

void FMWindow::refresh()
{
    if (m_operation_minimum_interval.isActive()) {
        return;
    }
    m_operation_minimum_interval.start(500);
    if (m_filter_visible)
    {
        advanceSearch();
        filterUpdate();
    }
    //qDebug() << "page refresh:" << getCurrentUri();
    //update page uri before refresh, fix go to root path issue after refresh
    m_tab->getActivePage()->getView()->setDirectoryUri(getCurrentUri());
    m_tab->getActivePage()->refresh();
}

void FMWindow::advanceSearch()
{
    //qDebug()<<"advanceSearch clicked";
    if (m_side_bar_container->currentWidget() == m_filter)
    {
        //clear filter
        filterUpdate();
        m_filter_bar->clearData();
        m_side_bar_container->setCurrentWidget(m_side_bar);
        m_filter_visible = false;
    }
    else
    {
        m_filter_visible = true;
        //before show, update cur uri
        QString target_path = getCurrentUri();
        if (target_path.contains("file://"))
            m_advance_target_path = target_path;
        else
            m_advance_target_path = "file://" + target_path;

        //set default search path as current path
        if (m_filter_bar)
        {
            m_filter_bar->setdefaultpath(target_path);
        }
        m_filter_bar->updateLocation();
        m_side_bar_container->setCurrentWidget(m_filter);
    }
    //to solve back up automatic pop up issue
    QTimer::singleShot(100, this, [=]() {
        m_search_bar->hideTableView();
    });
}

void FMWindow::clearRecord()
{
    //qDebug()<<"clearRecord clicked";
    m_search_bar->clearSearchRecord();
    m_clear_record->setDisabled(true);
}

void FMWindow::searchFilter(QString target_path, QString keyWord, bool search_file_name, bool search_content)
{
    auto targetUri = SearchVFSUriParser::parseSearchKey(target_path, keyWord, search_file_name, search_content);
    //qDebug()<<"targeturi:"<<targetUri;
    m_update_condition = true;
    this->goToUri(targetUri, true);
}

void FMWindow::filterUpdate(int type_index, int time_index, int size_index)
{
    //qDebug()<<"filterUpdate:";
    m_tab->getActivePage()->setSortFilter(type_index, time_index, size_index);
}

void FMWindow::setShowHidden(bool showHidden)
{
    //qDebug()<<"setShowHidden"<<m_show_hidden_file;
    m_show_hidden_file = !m_show_hidden_file;
    m_tab->getActivePage()->setShowHidden(m_show_hidden_file);
}

void FMWindow::setUseDefaultNameSortOrder(bool use)
{
    m_use_default_name_sort_order = ! m_use_default_name_sort_order;
    getCurrentPage()->setUseDefaultNameSortOrder(m_use_default_name_sort_order);
}

void FMWindow::setSortFolderFirst(bool set)
{
    m_folder_first = ! m_folder_first;
    getCurrentPage()->setSortFolderFirst(m_folder_first);
}

void FMWindow::forceStopLoading()
{
    m_tab->getActivePage()->stopLoading();
    m_is_loading = false;
}

void FMWindow::onPreviewPageSwitch(const QString &id)
{
    if (id.isNull()) {
        PreviewPageIface *page = m_preview_page_container->getCurrentPage();
        if (page) {
            m_preview_page_container->removePage(page);
        }
    } else {
        auto oldPage = m_preview_page_container->getCurrentPage();
        PreviewPageIface *page = PreviewPageFactoryManager::getInstance()->getPlugin(id)->createPreviewPage();
        m_preview_page_container->setCurrentPage(page);
        m_preview_page_container->removePage(oldPage);
        //emit selection changed signal manually for starting a preview with new page.
        Q_EMIT m_tab->currentSelectionChanged();
    }
}

void FMWindow::setCurrentSelectionUris(const QStringList &uris)
{
    m_tab->getActivePage()->getView()->setSelections(uris);
    m_tool_bar->updateStates();
}

void FMWindow::setCurrentSortOrder(Qt::SortOrder order)
{
    m_tab->getActivePage()->getView()->setSortOrder(order);
    m_tool_bar->updateStates();
}

Qt::SortOrder FMWindow::getCurrentSortOrder()
{
    return Qt::SortOrder(m_tab->getActivePage()->getView()->getSortOrder());
}

void FMWindow::setCurrentSortColumn(int column)
{
    m_tab->getActivePage()->getView()->setSortType(column);
}

int FMWindow::getCurrentSortColumn()
{
    return m_tab->getActivePage()->getView()->getSortType();
}

const QString FMWindow::getCurrentPageViewType()
{
    if (m_tab->getActivePage()->getView()) {
        return m_tab->getActivePage()->getView()->viewId();
    }
    else {
        return DirectoryViewFactoryManager2::getInstance()->getDefaultViewId();
    }
}

void FMWindow::editUri(const QString &uri)
{
    getCurrentPage()->getView()->editUri(uri);
}

void FMWindow::editUris(const QStringList &uris)
{
    getCurrentPage()->getView()->editUris(uris);
}

void FMWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        if (m_navigation_bar->isPathEditing()) {
            m_navigation_bar->finishEdit();
        } else {
            auto selections = this->getCurrentSelections();
            if (selections.count() == 1) {
                Q_EMIT m_tab->getActivePage()->viewDoubleClicked(selections.first());
            } else {
                QStringList files;
                QStringList dirs;
                for (auto uri : selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (info->isDir() || info->isVolume()) {
                        dirs<<uri;
                    } else {
                        files<<uri;
                    }
                }
                for (auto uri : dirs) {
                    m_tab->addPage(uri);
                }
                for (auto uri : files) {
                    Q_EMIT m_tab->getActivePage()->viewDoubleClicked(uri);
                }
            }
        }
    }
    return QMainWindow::keyPressEvent(e);
}

//preview page container
PreviewPageContainer::PreviewPageContainer(QWidget *parent) : QStackedWidget (parent)
{
    setMinimumWidth(300);
}

void PreviewPageContainer::setCurrentPage(PreviewPageIface *page)
{
    if (count() > 0) {
        PreviewPageIface *oldPage = getCurrentPage();
        if (oldPage) {
            removePage(oldPage);
        }
    }

    addWidget(dynamic_cast<QWidget*>(page));
    setCurrentWidget(dynamic_cast<QWidget*>(page));
    dynamic_cast<QWidget*>(page)->show();
    show();
}

void PreviewPageContainer::removePage(PreviewPageIface *page)
{
    if (!page)
        return;
    removeWidget(dynamic_cast<QWidget*>(page));
    if (count() == 0) {
        hide();
    }
    page->closePreviewPage();
}

PreviewPageIface *PreviewPageContainer::getCurrentPage()
{
    return dynamic_cast<PreviewPageIface*>(currentWidget());
}

//ToolBarContainer
ToolBarContainer::ToolBarContainer(QWidget *parent) : QToolBar(parent)
{

}

void ToolBarContainer::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setPen(this->palette().dark().color());
    p.drawLine(this->rect().bottomLeft(), this->rect().bottomRight());
}
