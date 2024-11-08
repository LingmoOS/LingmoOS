/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "header-bar.h"
#include "main-window.h"
#include <gio/gio.h>

#include "view-type-menu.h"
#include "sort-type-menu.h"
#include "operation-menu.h"

#include "directory-view-container.h"
#include "directory-view-widget.h"
#include "advanced-location-bar.h"

#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface2.h"
#include "file-info.h"
#include "file-info-job.h"

#include <QHBoxLayout>
#include <QUrl>
#include <QMessageBox>
#include <QProcess>
#include <QDir>

#include <QStyleOptionToolButton>

#include <QEvent>
#include <QApplication>
#include <QTimer>
#include <QStandardPaths>

#include <KWindowSystem>

#include <QtConcurrent>

#include <QX11Info>

#include <QPainter>

#include <QDebug>

static HeaderBarStyle *global_instance = nullptr;
static QString terminal_cmd = nullptr;

HeaderBar::HeaderBar(MainWindow *parent) : QToolBar(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    setMouseTracking(true);
    setStyle(HeaderBarStyle::getStyle());
    setFocusPolicy(Qt::TabFocus);

    m_window = parent;
    //disable default menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    //setAttribute(Qt::WA_OpaquePaintEvent);
    setStyleSheet(".HeaderBar{"
                  "background-color: transparent;"
                  "border: 0px solid transparent;"
                  "margin: 4px 5px 4px 5px;"
                  "}");

    setMovable(false);

//    auto a = addAction(QIcon::fromTheme("folder-new-symbolic"), tr("Create Folder"), [=]() {
//        //use the same function
//        m_window->createFolderOperation();
//    });
//    auto createFolder = qobject_cast<QToolButton *>(widgetForAction(a));
//    createFolder->setAutoRaise(false);
//    createFolder->setFixedSize(QSize(40, 40));
//    createFolder->setIconSize(QSize(16, 16));
//    m_create_folder = createFolder;

    //use theme buttons
//    auto createFolder = new HeadBarPushButton(this);
//    m_create_folder = createFolder;
//    createFolder->setFixedSize(QSize(40, 40));
//    createFolder->setIconSize(QSize(16, 16));
//    createFolder->setIcon(QIcon::fromTheme("folder-new-symbolic"));
//    createFolder->setToolTip(tr("Create Folder"));
//    addWidget(createFolder);
//    connect(createFolder, &QPushButton::clicked, m_window, [=]() {
//        //use the same function
//        m_window->createFolderOperation();
//    });

//    addSpacing(2);

    //find a terminal when init
//    findDefaultTerminal();
//    a = addAction(QIcon::fromTheme("terminal-app-symbolic"), tr("Open Terminal"), [=]() {
//        //open the default terminal
//        openDefaultTerminal();
//    });
//    auto openTerminal = qobject_cast<QToolButton *>(widgetForAction(a));
//    openTerminal->setAutoRaise(false);
//    openTerminal->setFixedSize(QSize(40, 40));
//    openTerminal->setIconSize(QSize(16, 16));

    //use theme buttons
//    auto openTerminal = new HeadBarPushButton(this);
//    openTerminal->setFixedSize(QSize(40, 40));
//    openTerminal->setIconSize(QSize(16, 16));
//    openTerminal->setIcon(QIcon::fromTheme("terminal-app-symbolic"));
//    openTerminal->setToolTip(tr("Open Terminal"));
//    addWidget(openTerminal);
//    connect(openTerminal, &QPushButton::clicked, m_window, [=]() {
//        //open the default terminal
//        openDefaultTerminal();
//    });

//    addSpacing(9 - 4);

    auto goBack = new QToolButton(this);
    m_go_back = goBack;
    goBack->setEnabled(false);
    goBack->setToolTip(tr("Go Back"));
    goBack->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    addWidget(goBack);

    auto goForward = new QToolButton(this);
    m_go_forward = goForward;
    goForward->setEnabled(false);
    goForward->setToolTip(tr("Go Forward"));
    goForward->setIcon(QIcon::fromTheme("go-next-symbolic"));
    addWidget(goForward);
    connect(goForward, &QToolButton::clicked, m_window, [=]() {
        m_window->getCurrentPage()->goForward();
    });

    addSpacing(ADDRESS_BAR_LEFT_WIDTH);

    auto locationBar = new Peony::AdvancedLocationBar(this);
    locationBar->setMinimumWidth(ADDRESS_BAR_MINIMUN_WIDTH);
    m_location_bar = locationBar;
    m_location_bar->setFocusPolicy(Qt::FocusPolicy(m_location_bar->focusPolicy() & ~Qt::TabFocus));
    addWidget(locationBar);

    connect(this, &HeaderBar::updateSearchProgress, m_location_bar, &Peony::AdvancedLocationBar::updateSearchProgress);
    connect(goBack, &QToolButton::clicked, m_window, [=]() {
        m_window->getCurrentPage()->goBack();
        quitSerachMode();
    });

    connect(m_location_bar, &Peony::AdvancedLocationBar::refreshRequest, [=]()
    {
        m_window->updateTabPageTitle();
    });
    connect(m_location_bar, &Peony::AdvancedLocationBar::updateFileTypeFilter, [=](const int &index) {
        m_window->getCurrentPage()->setSortFilter(index);
    });
    connect(m_location_bar, &Peony::AdvancedLocationBar::searchRequest, [=](const QString &path, const QString &key){
//        m_window->forceStopLoading();
//        //key is null, clean search content, show all files
//        if (key == "" || key.isNull())
//            Q_EMIT this->updateLocationRequest(path, true);
//        else
//        {
//            auto targetUri = Peony::SearchVFSUriParser::parseSearchKey(path, key, true, false, "", m_search_recursive);
//            Q_EMIT this->updateLocationRequest(targetUri, true);
//        }

        //search update all in main window
        m_window->forceStopLoading();
        Q_EMIT this->updateSearch(path, key, true);
    });

    connect(m_location_bar, &Peony::AdvancedLocationBar::updateWindowLocationRequest, this, &HeaderBar::updateLocationRequest);
//    connect(this, &HeaderBar::clearTrash, m_location_bar, &Peony::AdvancedLocationBar::clearTrash);
    addSpacing(ADDRESS_BAR_RIGHT_WIDTH);
//    auto a = addAction(QIcon::fromTheme("edit-find-symbolic"), tr("Search"));
//    connect(a, &QAction::triggered, this, &HeaderBar::searchButtonClicked);
//    auto search = qobject_cast<QToolButton *>(widgetForAction(a));
//    search->setAutoRaise(false);
//    search->setFixedSize(QSize(40, 40));
//    setIconSize(QSize(16, 16));
//    m_search_button = search;

    //use theme buttons
    auto search = new QToolButton(this);
    m_search_button = search;
    search->setIcon(QIcon::fromTheme("edit-find-symbolic"));
    search->setToolTip(tr("Search"));
    addWidget(search);
    connect(search, &QToolButton::clicked, this, &HeaderBar::searchButtonClicked);

//    createFolder->setFlat(true);
//    createFolder->setProperty("isWindowButton", 1);
//    createFolder->setProperty("useIconHighlightEffect", 2);
//    createFolder->setProperty("isIcon", true);

//    openTerminal->setFlat(true);
//    openTerminal->setProperty("isWindowButton", 1);
//    openTerminal->setProperty("useIconHighlightEffect", 2);
//    openTerminal->setProperty("isIcon", true);

    //fix:bug#93521,修改控件三态
    search->setProperty("useIconHighlightEffect", true);
    search->setProperty("iconHighlightEffectMode", 1);

    goBack->setProperty("isWindowButton", 1);
    goBack->setProperty("useIconHighlightEffect", 2);
    goBack->setProperty("isIcon", true);

    goForward->setProperty("isWindowButton", 1);
    goForward->setProperty("useIconHighlightEffect", 2);
    goForward->setProperty("isIcon", true);

    addMenuButtons();
    setMinimumWidth(GBACK_BTN_WIDTH * 2
                    + ADDRESS_BAR_LEFT_WIDTH + ADDRESS_BAR_MINIMUN_WIDTH + ADDRESS_BAR_RIGHT_WIDTH
                    + SEARCH_BTN_WIDTH
                    + DRAG_AREA_MINIMUN_WIDTH);
//    m_focus_list<<(createFolder);
//    m_focus_list<<(openTerminal);
    m_focus_list<<(goBack);
    m_focus_list<<(goForward);
    m_focus_list<<(search);
}

void HeaderBar::findDefaultTerminal()
{
    QtConcurrent::run([](){
        GList *infos = g_app_info_get_all();
        GList *l = infos;
        while (l) {
            const char *cmd = g_app_info_get_executable(static_cast<GAppInfo*>(l->data));
            QString tmp = cmd;
            if (tmp.contains("terminal")) {
                terminal_cmd = tmp;
                if (tmp == "mate-terminal") {
                    terminal_cmd = "/usr/bin/mate-terminal";
                    break;
                }
            }
            l = l->next;
        }
        g_list_free_full(infos, g_object_unref);
    });
}

void HeaderBar::openDefaultTerminal()
{
    //don't find any terminal
    if (terminal_cmd == nullptr)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Operate Tips"));
        msgBox->setText(tr("Don't find any terminal, please install at least one terminal!"));
        msgBox->exec();
        return;
    }

    QUrl url = m_window->getCurrentUri();
    auto directory = url.path().toUtf8().constData();
    gchar **argv = nullptr;
    g_shell_parse_argv (terminal_cmd.toUtf8().constData(), nullptr, &argv, nullptr);
    GError *err = nullptr;
    g_spawn_async (directory,
                   argv,
                   nullptr,
                   G_SPAWN_SEARCH_PATH,
                   nullptr,
                   nullptr,
                   nullptr,
                   &err);
    if (err) {
        qDebug()<<err->message;
        g_error_free(err);
        err = nullptr;
        tryOpenAgain();
    }
    g_strfreev (argv);
}

void HeaderBar::tryOpenAgain()
{
    QUrl url = m_window->getCurrentUri();
    auto absPath = url.path();
    qDebug() << "tryOpenAgain url" <<url<<absPath<<terminal_cmd;
    QProcess p;
    p.setProgram(terminal_cmd);
    p.setArguments(QStringList()<<"--working-directory"<<absPath);
    p.startDetached(p.program(), p.arguments());
    p.waitForFinished(-1);
}

void HeaderBar::searchButtonClicked()
{
    m_search_mode = ! m_search_mode;
    qDebug() << "searchButtonClicked" <<m_search_mode;
    m_window->updateSearchStatus(m_search_mode);
}

void HeaderBar::setSearchMode(bool mode)
{
    m_search_button->setCheckable(mode);
    m_search_button->setChecked(mode);
    m_search_button->setDown(mode);
    if (! mode)
        m_search_button->setFocusPolicy(Qt::NoFocus);
    else
        m_search_button->setFocusPolicy(Qt::ClickFocus);
    m_location_bar->switchEditMode(mode);
}

void HeaderBar::closeSearch()
{
    m_search_mode = false;
    setSearchMode(false);
}

void HeaderBar::initFocus()
{
    for (auto widget : m_focus_list) {
        widget->setFocusPolicy(Qt::StrongFocus);
        m_window->addFocusWidgetToFocusList(widget);
    }
}

void HeaderBar::updateSearchRecursive(bool recursive)
{
    m_search_recursive = recursive;
}

void HeaderBar::addSpacing(int pixel)
{
    QWidget *widget = new QWidget(this);
    widget->setFixedSize(QSize((pixel < 1 ? 1 : pixel), 40));
    widget->setAttribute(Qt::WA_TranslucentBackground);

    addWidget(widget);
//    for (int i = 0; i < pixel; i++) {
//        addSeparator();
//    }
}

void HeaderBar::addMenuButtons()
{
    //占位widget,5个widget共同组成拖动区域,共宽150px
    for (int i = 1; i <= 5; ++i) {
        QWidget *widget = new QWidget(this);
        widget->setFixedSize(QSize(i * 10, 40));
        widget->setAttribute(Qt::WA_TranslucentBackground);
        addWidget(widget);
    }

    QAction *action = addAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"));
    QToolButton *viewType = qobject_cast<QToolButton *>(widgetForAction(action));
    viewType->setAutoRaise(true);
    viewType->setFixedWidth(57);
    viewType->setPopupMode(QToolButton::InstantPopup);

    ViewTypeMenu* viewTypeMenu = new ViewTypeMenu(viewType);
    m_view_type_menu = viewTypeMenu;
    action->setMenu(viewTypeMenu);

    connect(viewTypeMenu, &ViewTypeMenu::switchViewRequest, this, [=](const QString &id, const QIcon &icon, bool resetToZoomLevel) {
        viewType->setText(id);
        viewType->setIcon(icon);
        viewTypeChangeRequest(id);
        if (resetToZoomLevel) {
            auto viewId = m_window->getCurrentPage()->getView()->viewId();
            auto factoryManger = Peony::DirectoryViewFactoryManager2::getInstance();
            auto factory = factoryManger->getFactory(viewId);
            int zoomLevelHint = factory->zoom_level_hint();
            m_window->getCurrentPage()->setZoomLevelRequest(zoomLevelHint);
        }
    });

    connect(viewTypeMenu, &ViewTypeMenu::updateZoomLevelHintRequest, this, &HeaderBar::updateZoomLevelHintRequest);

    action = addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), tr("Sort Type"));
    QToolButton *sortType = qobject_cast<QToolButton *>(widgetForAction(action));
    sortType->setAutoRaise(true);
    sortType->setFixedWidth(57);
    sortType->setPopupMode(QToolButton::InstantPopup);

    SortTypeMenu *sortTypeMenu = new SortTypeMenu(sortType);
    m_sort_type_menu = sortTypeMenu;
    action->setMenu(sortTypeMenu);

    connect(sortTypeMenu, &SortTypeMenu::switchSortTypeRequest, m_window, &MainWindow::setCurrentSortColumn);
    connect(sortTypeMenu, &SortTypeMenu::switchSortOrderRequest, m_window, [=](Qt::SortOrder order) {
        if (order == Qt::AscendingOrder) {
            sortType->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
        } else {
            sortType->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
        }
        m_window->setCurrentSortOrder(order);
    });
    connect(sortTypeMenu, &QMenu::aboutToShow, sortTypeMenu, [=]() {
        bool originPathVisible = m_window->getCurrentUri() == "trash:///";
        sortTypeMenu->setOriginPathVisible(originPathVisible);
        sortTypeMenu->setSortType(m_window->getCurrentSortColumn());
        sortTypeMenu->setSortOrder(m_window->getCurrentSortOrder());
    });
    connect(sortTypeMenu, &SortTypeMenu::globalSortingPolicyChanged, this, [=]{
        auto sortType = m_window->getCurrentSortColumn();
        auto sortOrder = m_window->getCurrentSortOrder();
        sortTypeMenu->setSortType(sortType);
        sortTypeMenu->setSortOrder(sortOrder);
        m_window->setCurrentSortColumn(sortType);
        m_window->setCurrentSortOrder(sortOrder);
    });

    action = addAction(QIcon::fromTheme("open-menu-symbolic"), tr("Option"));
    QToolButton *popMenu = qobject_cast<QToolButton *>(widgetForAction(action));
    popMenu->setAutoRaise(false);
    popMenu->setProperty("isOptionButton", true);
    popMenu->setPopupMode(QToolButton::InstantPopup);

    OperationMenu *operationMenu = new OperationMenu(m_window, popMenu);
    m_operation_menu = operationMenu;
    action->setMenu(operationMenu);

    viewType->setProperty("isWindowButton", 1);
    viewType->setProperty("useIconHighlightEffect", 0x2);
    viewType->setAutoRaise(true);

    sortType->setProperty("isWindowButton", 1);
    sortType->setProperty("useIconHighlightEffect", 0x2);
    sortType->setAutoRaise(true);

    popMenu->setProperty("isWindowButton", 1);
    popMenu->setProperty("useIconHighlightEffect", 0x2);
    popMenu->setAutoRaise(true);

    m_focus_list << (viewType);
    m_focus_list << (sortType);
    m_focus_list << (popMenu);
}

void HeaderBar::mouseMoveEvent(QMouseEvent *e)
{
    QToolBar::mouseMoveEvent(e);
    QCursor c;
    c.setShape(Qt::ArrowCursor);
    this->topLevelWidget()->setCursor(c);
}

//fix Right click in the blank space on the right side of the tab to quickly maximize and restore the window. link bug102455
void HeaderBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    QToolBar::mouseDoubleClickEvent(e);
    if(e->button() == Qt::LeftButton || e->button() == Qt::RightButton){
        m_window->maximizeOrRestore();
    }
}

void HeaderBar::setLocation(const QString &uri)
{
    m_location_bar->updateLocation(uri);
}

void HeaderBar::cancelEdit()
{
    m_location_bar->cancelEdit();
}

void HeaderBar::startEdit(bool bSearch)
{
    //qDebug() << "bSearch" <<bSearch <<m_search_mode;
    if (bSearch && m_search_mode)
        return;

    if (bSearch)
    {
        searchButtonClicked();
    }
    else
    {
        m_search_mode = false;
        m_location_bar->startEdit();
        m_location_bar->switchEditMode(false);
    }
}

void HeaderBar::finishEdit()
{
    m_location_bar->finishEdit();
}

void HeaderBar::quitSerachMode()
{
    if (m_search_mode)
       m_location_bar->clearSearchBox();
}

void HeaderBar::updateIcons()
{
    if(!m_window)
        return;
    qDebug()<<"updateIcons:" <<m_window->getCurrentUri();
    qDebug()<<"updateIcons:" <<m_window->getCurrentSortColumn();
    qDebug()<<"updateIcons:" <<m_window->getCurrentSortOrder();
    m_view_type_menu->setCurrentDirectory(m_window->getCurrentUri());
    if(m_window->getCurrentPage() && m_window->getCurrentPage()->getView())
        m_view_type_menu->setCurrentView(m_window->getCurrentPage()->getView()->viewId(), true);
    m_sort_type_menu->switchSortTypeRequest(m_window->getCurrentSortColumn());
    m_sort_type_menu->switchSortOrderRequest(m_window->getCurrentSortOrder());

    //go back & go forward
    if(m_window->getCurrentPage()){
        m_go_back->setEnabled(m_window->getCurrentPage()->canGoBack());
        m_go_forward->setEnabled(m_window->getCurrentPage()->canGoForward());
    }

    //fix create folder fail issue in special path
//    auto curUri = m_window->getCurrentUri();
//    auto info = Peony::FileInfo::fromUri(curUri);
    //Peony::FileInfoJob job(info);
    //job.querySync();
//    if (info->canWrite())
//        m_create_folder->setEnabled(true);
//    else
//        m_create_folder->setEnabled(false);

    //maximize & restore
    //updateMaximizeState();
}

void HeaderBar::updateViewTypeEnable()
{
    auto url = m_window->getCurrentUri();
    //qDebug() << "updateViewTypeEnable url:" << url;
    if(url == "computer:///"){
        m_view_type_menu->setEnabled(false);
        m_view_type_menu->menuAction()->setVisible(false);
    }else{
        m_view_type_menu->setEnabled(true);
        m_view_type_menu->menuAction()->setVisible(true);
    }
}

void HeaderBar::updateHeaderState()
{
//    if (! m_window->getFilterWorking())
//        m_create_folder->setEnabled(true);
//    else
//        m_create_folder->setEnabled(false);
}

void HeaderBar::updateSortTypeEnable()
{
    auto url = m_window->getCurrentUri();
    qDebug() << "url:" << url;
    if(url == "computer:///"){
        m_sort_type_menu->setEnabled(false);
        m_sort_type_menu->menuAction()->setVisible(false);
    }else{
        m_sort_type_menu->setEnabled(true);
        m_sort_type_menu->menuAction()->setVisible(true);
    }
}

void HeaderBar::updateMaximizeState()
{
    //maximize & restore
    bool maximized = m_window->isMaximized();
    if (maximized) {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maximize_restore_button->setToolTip(tr("Restore"));
    } else {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maximize_restore_button->setToolTip(tr("Maximize"));
    }
}

//HeaderBarToolButton
HeaderBarToolButton::HeaderBarToolButton(QWidget *parent) : QToolButton(parent)
{
    setAutoRaise(false);
    setIconSize(QSize(16, 16));
}

//HeadBarPushButton
HeadBarPushButton::HeadBarPushButton(QWidget *parent) : QPushButton(parent)
{
    setIconSize(QSize(16, 16));
}

//HeaderBarStyle
HeaderBarStyle *HeaderBarStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new HeaderBarStyle;
    }
    return global_instance;
}

int HeaderBarStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (qobject_cast<const HeaderBarContainer *>(widget))
        return 0;

    switch (metric) {
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarSeparatorExtent:
        return 1;
    case PM_ToolBarItemSpacing: {
        return 1;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void HeaderBarStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    //This is a "lie". We want to use instant popup menu for tool button, and we aslo
    //want use popup menu style with this tool button, so we change the related flags
    //to draw in our expected.
    if (control == CC_ToolButton) {
        QStyleOptionToolButton button = *qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (button.features.testFlag(QStyleOptionToolButton::HasMenu)) {
            button.features = QStyleOptionToolButton::None;
            if (!widget->property("isOptionButton").toBool()) {
                button.features |= QStyleOptionToolButton::HasMenu;
                button.features |= QStyleOptionToolButton::MenuButtonPopup;
                button.subControls |= QStyle::SC_ToolButtonMenu;
            }
            return qApp->style()->drawComplexControl(control, &button, painter, widget);
        }
    }
    return qApp->style()->drawComplexControl(control, option, painter, widget);
}

void HeaderBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_IndicatorToolBarSeparator) {
        return;
    }
    return qApp->style()->drawPrimitive(element, option, painter, widget);
}

HeaderBarContainer::HeaderBarContainer(QWidget *parent) : QToolBar(parent)
{
    setStyle(HeaderBarStyle::getStyle());

    setContextMenuPolicy(Qt::CustomContextMenu);

    setStyleSheet(".HeaderBarContainer"
                  "{"
                  "background-color: transparent;"
                  "border: 0px solid transparent"
                  "}");

    setFixedHeight(50);
    setMovable(false);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_internal_widget = new QWidget(this);
    m_internal_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

bool HeaderBarContainer::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    auto window = qobject_cast<MainWindow *>(obj);
    if (window) {
        //use updateMaximizeState function, comment those code
        if (e->type() == QEvent::Resize || QEvent::WindowStateChange == e->type()) {
            //fix bug#95419, drag change window to restore not change state issue
            m_header_bar->updateMaximizeState();
        }
        //fix double click space window has no change issue, bug#38499
        //fix Right click in the blank space on the right side of the tab to quickly maximize and restore the window. link bug102455
//        if (e->type() == QEvent::MouseButtonDblClick)
//        {
//            m_header_bar->m_window->maximizeOrRestore();
//        }
        return false;
    } else {
        if (e->type() == QEvent::MouseMove) {
            //auto w = qobject_cast<QWidget *>(obj);
            QCursor c;
            c.setShape(Qt::ArrowCursor);
            //this->setCursor(c);
            //w->setCursor(c);
            this->topLevelWidget()->setCursor(c);
        }

    }

    return false;
}

void HeaderBarContainer::addHeaderBar(HeaderBar *headerBar)
{
    if (m_header_bar)
        return;
    m_header_bar = headerBar;

    headerBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(headerBar);

    headerBar->initFocus();

    addWindowButtons();

    m_internal_widget->setLayout(m_layout);
    addWidget(m_internal_widget);

    m_header_bar->m_window->installEventFilter(this);
}

void HeaderBarContainer::addWindowButtons()
{
    //m_window_buttons = new QWidget(this);
    auto layout = new QHBoxLayout;

    layout->setContentsMargins(0, 0, 4, 0);
    layout->setSpacing(4);

    //minimize, maximize and close
    auto minimize = new QToolButton(m_internal_widget);
    minimize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimize->setToolTip(tr("Minimize"));
    minimize->setAutoRaise(false);
    connect(minimize, &QToolButton::clicked, this, [=]() {
        KWindowSystem::minimizeWindow(m_header_bar->m_window->winId());
        //m_header_bar->m_window->showMinimized();
    });

    //window-maximize-symbolic
    //window-restore-symbolic
    auto maximizeAndRestore = new QToolButton(m_internal_widget);
    m_header_bar->m_maximize_restore_button = maximizeAndRestore;
    //switch tips with button status, fix bug#77604
    m_header_bar->updateMaximizeState();
    maximizeAndRestore->setAutoRaise(false);
    connect(maximizeAndRestore, &QToolButton::clicked, this, [=]() {
        m_header_bar->m_window->maximizeOrRestore();
    });
    m_max_or_restore = maximizeAndRestore;

    auto close = new QToolButton(m_internal_widget);
    close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close->setToolTip(tr("Close"));
    close->setAutoRaise(false);

    minimize->setProperty("isWindowButton", 1);
    minimize->setProperty("useIconHighlightEffect", 0x2);
    minimize->setAutoRaise(true);

    maximizeAndRestore->setProperty("isWindowButton", 1);
    maximizeAndRestore->setProperty("useIconHighlightEffect", 0x2);
    maximizeAndRestore->setAutoRaise(true);

    close->setProperty("isWindowButton", 2);
    close->setProperty("useIconHighlightEffect", 0x8);
    close->setAutoRaise(true);

    connect(close, &QToolButton::clicked, this, [=]() {
        m_header_bar->m_window->close();
    });

    //fix bug#98500, comment self change palette code, use platform-theme way
//    connect(qApp, &QApplication::paletteChanged, close, [=](){
//        QTimer::singleShot(100, this, [=](){
//            auto palette = qApp->palette();
//            palette.setColor(QPalette::Highlight, QColor("#E54A50"));
//            close->setPalette(palette);
//        });
//    });
//    auto palette = qApp->palette();
//    palette.setColor(QPalette::Highlight, QColor("#E54A50"));
//    close->setPalette(palette);

    layout->addWidget(minimize);
    layout->addWidget(maximizeAndRestore);
    layout->addWidget(close);

    m_layout->addLayout(layout);

    minimize->setMouseTracking(true);
    minimize->installEventFilter(this);
    maximizeAndRestore->setMouseTracking(true);
    maximizeAndRestore->installEventFilter(this);
    close->setMouseTracking(true);
    close->installEventFilter(this);

    if (!QX11Info::isPlatformX11()) {
        minimize->setVisible(false);
        maximizeAndRestore->setVisible(false);
        close->setVisible(false);
        m_layout->removeItem(layout);
    }

    m_header_bar->m_window->addFocusWidgetToFocusList(minimize);
    m_header_bar->m_window->addFocusWidgetToFocusList(maximizeAndRestore);
    m_header_bar->m_window->addFocusWidgetToFocusList(close);
}

void HeaderBarContainer::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyleOptionToolBar opt;
    initStyleOption(&opt);
    bool isEnable = opt.state & QStyle::State_Enabled;
    bool isActive = opt.state & QStyle::State_Active;
    auto color = qApp->palette().color(isEnable? isActive? QPalette::Active: QPalette::Inactive: QPalette::Disabled, QPalette::Base);
    p.fillRect(this->rect(), color);
}
