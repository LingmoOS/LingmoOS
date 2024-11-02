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

#include "main-window.h"
#include "header-bar.h"
#include "global-settings.h"

#include "border-shadow-effect.h"
#include "directoryviewhelper.h"
#include <KWindowSystem>
#include <QVariant>
#include <QMouseEvent>
#include <QX11Info>

#include <QDockWidget>
#include <QTreeView>

#include <QScreen>

#include "side-bar-proxy-filter-sort-model.h"
#include "side-bar-model.h"

#include "directory-view-container.h"
#include "tab-widget.h"
#include "tab-status-bar.h"
#include "x11-window-manager.h"
#include "properties-window-factory-plugin-manager.h"
//#include "properties-window.h"
#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"

#include "navigation-side-bar.h"
#include "advance-search-bar.h"
#include "status-bar.h"

#include "explor-main-window-style.h"

#include "file-operation-manager.h"
#include "file-operation-utils.h"
#include "file-utils.h"
#include "create-template-operation.h"
#include "file-operation-error-dialog.h"
#include "clipboard-utils.h"
#include "search-vfs-uri-parser.h"
#include "file-delete-operation.h"

#include "directory-view-menu.h"
#include "directory-view-widget.h"
#include "main-window-factory.h"
#include "thumbnail-manager.h"

#include "../explor-application.h"

#include "global-settings.h"
#include "audio-play-manager.h"

#include "float-pane-widget.h"

#include "file-meta-info.h"
#include "sound-effect.h"
#include "location-bar.h"
#include "file-launch-action.h"
#include "file-launch-manager.h"
#include <QSplitter>

#include <QPainter>

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>
#include <QDesktopServices>

#include <QProcess>
#include <QDateTime>

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QPainterPath>
#endif

#include <QDebug>

#include <X11/Xlib.h>
#include <KWindowEffects>
#include <netwm.h>

// NOTE build failed on Archlinux. Can't detect `QGSettings/QGSettings' header
// fixed by replaced `QGSettings/QGSettings' with `QGSettings'
#include <QGSettings>

#include <QDBusConnection>
#include <QDBusReply>

static MainWindow *last_resize_window = nullptr;

static QWidgetList blur_window_list;

MainWindow::MainWindow(const QString &uri, QWidget *parent) : QMainWindow(parent)
{ 
    // try fix #162452, filedialog changes explor main windows view type and sort options.
    setObjectName("_explor_mainwindow");

    setContextMenuPolicy(Qt::CustomContextMenu);
    installEventFilter(this);

    //use qApp set window icon, task#29435
    qApp->setWindowIcon(QIcon::fromTheme("system-file-manager"));
    //setWindowTitle(tr("File Manager"));

    //check all settings and init
    checkSettings();

    setStyle(PeonyMainWindowStyle::getStyle());

    m_effect = new BorderShadowEffect(this);
    m_effect->setPadding(0);
    m_effect->setBorderRadius(0);
    m_effect->setBlurRadius(0);
    //setGraphicsEffect(m_effect);

    setAnimated(false);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_OpaquePaintEvent);
    //fix double window base buttons issue, not effect MinMax button hints
    //auto flags = windowFlags() &~Qt::WindowMinMaxButtonsHint;
    //setWindowFlags(flags |Qt::FramelessWindowHint);
    //setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    //setContentsMargins(4, 4, 4, 4);

    //disable style window manager
    setProperty("useStyleWindowManager", false);

    //set minimum width by design request
    setMinimumWidth(WINDOW_MINIMUM_WIDTH);

    //init UI
    initUI(uri);

    // set tab order

    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setLINGMODecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
        NETWinInfo info(QX11Info::connection(), this->winId(), QX11Info::appRootWindow(), NET::Properties(), NET::WM2DesktopFileName);
        info.setDesktopFileName("explor");
    }

    startMonitorThumbnailForbidStatus();

    auto start_cost_time = QDateTime::currentMSecsSinceEpoch() - qApp->property("startTime").toLongLong();//PeonyApplication::explor_start_time;
    qDebug() << "explor start end in main-window time:" <<start_cost_time
             <<"ms"<<QDateTime::currentMSecsSinceEpoch();

    if (blur_window_list.count() < 5) {
        blur_window_list.append(this);
        m_is_blur_window = true;
        KWindowEffects::enableBlurBehind(winId(), true);
    }

#ifdef LINGMO_SDK_DATE
    connect(Peony::GlobalSettings::getInstance(),
            &Peony::GlobalSettings::updateShortDataFormat,
            this,
            &MainWindow::updateDateFormat);
#endif
    connect(this, &MainWindow::setSelection, [=](QStringList selectionUris){
        QTimer::singleShot(500, [=] {
            getCurrentPage()->getView()->setSelections(selectionUris);
            getCurrentPage()->getView()->scrollToSelection(selectionUris.first());
        });
    });
}

MainWindow::~MainWindow()
{
    blur_window_list.removeOne(this);

    //fix bug 40913, when window is maximazed, not update size
    if (last_resize_window == this && !isMaximized()) {
        auto settings = Peony::GlobalSettings::getInstance();
        settings->setValue(DEFAULT_WINDOW_SIZE, this->size());
        last_resize_window = nullptr;
    }
}

void MainWindow::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        this->refresh();
        m_date_format = dateFormat;
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        auto me = static_cast<QMouseEvent *>(event);
        auto widget = this->childAt(me->pos());
        if (!widget) {
            // set default sidebar width flag
            m_should_save_side_bar_width = true;
        }
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        if (m_should_save_side_bar_width) {
            // real set default side bar width
            auto settings = Peony::GlobalSettings::getInstance();
            settings->setValue(DEFAULT_SIDEBAR_WIDTH, m_side_bar->width());
            qDebug() << "main window set DEFAULT_SIDEBAR_WIDTH:" << m_side_bar->width();
        }
        m_should_save_side_bar_width = false;
    }

    return false;
}

QSize MainWindow::sizeHint() const
{
    auto screenSize = QApplication::primaryScreen()->size();
    QSize windowSize;
    windowSize.setWidth(screenSize.width()*2/3);
    windowSize.setHeight(screenSize.height()*4/5);
    QSize defaultSize = (Peony::GlobalSettings::getInstance()->getValue(DEFAULT_WINDOW_SIZE)).toSize();
    if (!defaultSize.isValid())
        return windowSize;
    int width = qMin(defaultSize.width(), screenSize.width());
    int height = qMin(defaultSize.height(), screenSize.height());

    //return screenSize*2/3;
    //qreal dpr = qApp->devicePixelRatio();
    return QSize(width, height);
}

Peony::FMWindowIface *MainWindow::create(const QString &uri)
{
    auto window = new MainWindow(uri);
    connect(window, &MainWindow::locationChanged, this, [=](){
        if (currentViewSupportZoom())
            window->setCurrentViewZoomLevel(this->currentViewZoomLevel());
    });
    window->setAttribute(Qt::WA_DeleteOnClose);
    KWindowSystem::raiseWindow(window->winId());
    if (KWindowSystem::activeWindow() != window->winId()) {
        KWindowSystem::activateWindow(window->winId());
    }
    window->show();
    return window;
}

Peony::FMWindowIface *MainWindow::create(const QStringList &uris)
{
    if (uris.isEmpty()) {
        auto window = new MainWindow;
        if (currentViewSupportZoom())
            window->setCurrentViewZoomLevel(this->currentViewZoomLevel());
        return window;
    }
    auto uri = uris.first();
    auto l = uris;
    l.removeAt(0);
    auto window = new MainWindow(uri);
    if (currentViewSupportZoom())
        window->setCurrentViewZoomLevel(this->currentViewZoomLevel());
    window->addNewTabs(l);
    window->setAttribute(Qt::WA_DeleteOnClose);
    KWindowSystem::raiseWindow(window->winId());
    if (KWindowSystem::activeWindow() != window->winId()) {
        KWindowSystem::activateWindow(window->winId());
    }
    window->show();
    return window;
}

Peony::FMWindowIface *MainWindow::createWithZoomLevel(const QString &uri, int zoomLevel)
{
    auto window = new MainWindow(uri);
    if (currentViewSupportZoom())
        window->setCurrentViewZoomLevel(zoomLevel);
    return window;
}

Peony::FMWindowIface *MainWindow::createWithZoomLevel(const QStringList &uris, int zoomLevel)
{
    if (uris.isEmpty()) {
        auto window = new MainWindow;
        if (currentViewSupportZoom())
            window->setCurrentViewZoomLevel(zoomLevel);
        return window;
    }
    auto uri = uris.first();
    auto l = uris;
    l.removeAt(0);
    auto window = new MainWindow(uri);
    if (currentViewSupportZoom())
        window->setCurrentViewZoomLevel(zoomLevel);
    window->addNewTabs(l);
    return window;
}

Peony::FMWindowFactory *MainWindow::getFactory()
{
    return nullptr;//StableMainWindowFactory::getInstance();
}

Peony::DirectoryViewContainer *MainWindow::getCurrentPage()
{
    return m_tab->currentPage();
}

void MainWindow::checkSettings()
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style"))
    {
        //font monitor
        QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
            qDebug() << "fontSetting changed:" << key;
            if (key == "systemFont" || key == "systemFontSize") {
                QFont font = this->font();
                for(auto widget : qApp->allWidgets())
                    widget->setFont(font);
            }
            //use qApp set window icon, task#29435
            /*else if ("iconThemeName" == key) {
                setWindowIcon(QIcon::fromTheme("system-file-manager"));
            }*/
        });
    }
}

void MainWindow::setShortCuts()
{
    if (!m_shortcuts_set) {
        //stop loading action
        QAction *stopLoadingAction = new QAction(this);
        stopLoadingAction->setShortcut(QKeySequence(Qt::Key_Escape));
        addAction(stopLoadingAction);
        connect(stopLoadingAction, &QAction::triggered, this, &MainWindow::forceStopLoading);

        //show hidden action
        QAction *showHiddenAction = new QAction(this);
        showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
        showHiddenAction->setCheckable(true);
        addAction(showHiddenAction);
        connect(showHiddenAction, &QAction::triggered, this, [=]() {
            //qDebug() << "show hidden";
            this->setShowHidden(!getWindowShowHidden());
        });

        auto undoAction = new QAction(QIcon::fromTheme("edit-undo-symbolic"), tr("Undo"), this);
        undoAction->setShortcut(QKeySequence::Undo);
        addAction(undoAction);
        connect(undoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->undo();
        });

        auto redoAction = new QAction(QIcon::fromTheme("edit-redo-symbolic"), tr("Redo"), this);
        redoAction->setShortcut(QKeySequence::Redo);
        addAction(redoAction);
        connect(redoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->redo();
        });

        //add CTRL+D for delete operation
        auto trashAction = new QAction(this);
        trashAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Delete<<QKeySequence(Qt::CTRL + Qt::Key_D));
        connect(trashAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://")
                    || currentUri.startsWith("favorite://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            if (!uris.isEmpty() && !uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {

                bool canTrash = true;
                for (auto uri : uris) {
                    if(Peony::FileUtils::isLongNameFileOfNotDel2Trash(uri)){/* 在家目录/下载/扩展目录下存放的长文件名文件使用永久删除，link bug#188864 */
                        canTrash = false;
                        break;
                    }
                }

                bool isTrash = this->getCurrentUri() == "trash:///";
                if (!isTrash && canTrash) {
                    Peony::FileOperationUtils::trash(uris, true);
                } else {
                    Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
                }
            }
        });
        addAction(trashAction);

        auto deleteAction = new QAction(this);
        deleteAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::SHIFT + Qt::Key_Delete));
        addAction(deleteAction);
        connect(deleteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            if (! uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {
                Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
            }
        });

        auto searchAction = new QAction(this);
        searchAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::Key_F));
        connect(searchAction, &QAction::triggered, this, [=]() {
            m_is_search = ! m_is_search;
            m_header_bar->startEdit(m_is_search);
        });
        addAction(searchAction);

        //F4 or Alt+D, change to address
        auto locationAction = new QAction(this);
        locationAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F4<<QKeySequence(Qt::ALT + Qt::Key_D));
        connect(locationAction, &QAction::triggered, this, [=]() {
            m_header_bar->startEdit();
        });
        addAction(locationAction);

        auto newWindowAction = new QAction(this);
        newWindowAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
        connect(newWindowAction, &QAction::triggered, this, [=]() {
            MainWindow *newWindow = new MainWindow(getCurrentUri());
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
            //PeonyApplication::about();
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
        previousTabAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab));
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
        newFolderAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
        connect(newFolderAction, &QAction::triggered, this, [=](){
            QString boxpath = "file://"+QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.box";
            if (boxpath == getCurrentUri()) {
                return;
            }
            //not allow create in phone path
            if (getCurrentUri().startsWith("mtp://") || getCurrentUri().startsWith("gphoto2://")){
                return;
            }

            createFolderOperation();
        });
        addAction(newFolderAction);

        //show selected item's properties
        auto propertiesWindowAction = new QAction(this);
        propertiesWindowAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_Return)
                                             <<QKeySequence(Qt::ALT + Qt::Key_Enter));
        connect(propertiesWindowAction, &QAction::triggered, this, [=]() {
            //Fixed issue:when use this shortcut without any selections, this will crash
            QStringList uris;
            if (getCurrentSelections().count() > 0)
            {
                uris<<getCurrentSelections();
            }
            else
            {
                uris<<getCurrentUri();
            }
            QMainWindow *w = Peony::PropertiesWindowFactoryPluginManager::getInstance()->create(uris);
            //Peony::PropertiesWindow *w = new Peony::PropertiesWindow(uris);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
        });
        addAction(propertiesWindowAction);

        auto helpAction = new QAction(this);
        helpAction->setShortcut(QKeySequence(Qt::Key_F1));
        connect(helpAction, &QAction::triggered, this, [=]() {
            //PeonyApplication::help();
        });
        addAction(helpAction);

        auto maxAction = new QAction(this);
        maxAction->setShortcut(QKeySequence(Qt::Key_F11));
        connect(maxAction, &QAction::triggered, this, [=]() {
            //showFullScreen has some issue, change to showMaximized, fix #20043
            m_header_bar->cancelEdit();
            maximizeOrRestore();
        });
        addAction(maxAction);

        auto previewPageAction = new QAction(this);
        previewPageAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F3<<QKeySequence(Qt::ALT + Qt::Key_P));
        connect(previewPageAction, &QAction::triggered, this, [=]() {
            auto triggered = m_tab->getTriggeredPreviewPage();
            if (triggered)
            {
                m_tab->setPreviewPage(nullptr);
            }
            else
            {
                auto instance = Peony::PreviewPageFactoryManager::getInstance();
                auto lastPreviewPageId  = instance->getLastPreviewPageId();
                auto *page = instance->getPlugin(lastPreviewPageId)->createPreviewPage();
                m_tab->setPreviewPage(page);
            }
            m_tab->setTriggeredPreviewPage(!triggered);
            m_tab->updatePreviewButtonStatus(!triggered);

        });
        addAction(previewPageAction);

        auto refreshWindowAction = new QAction(this);
        refreshWindowAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        connect(refreshWindowAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshWindowAction);

        auto listToIconViewAction = new QAction(this);
        listToIconViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
        connect(listToIconViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("Icon View"));
        });
        addAction(listToIconViewAction);

        auto iconToListViewAction = new QAction(this);
        iconToListViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
        connect(iconToListViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("List View"));
        });
        addAction(iconToListViewAction);

        auto reverseSelectAction = new QAction(this);
        reverseSelectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
        connect(reverseSelectAction, &QAction::triggered, this, [=]() {
            this->getCurrentPage()->getView()->invertSelections();
        });
        addAction(reverseSelectAction);

        auto remodelViewAction = new QAction(this);
        remodelViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
        connect(remodelViewAction, &QAction::triggered, this, [=]() {
            this->getCurrentPage()->setZoomLevelRequest(25);
        });
        addAction(remodelViewAction);

        auto enlargViewAction = new QAction(this);
        enlargViewAction->setShortcut(QKeySequence::ZoomIn);
        connect(enlargViewAction, &QAction::triggered, this, [=]() {
            int defaultZoomLevel = this->currentViewZoomLevel();
            if(defaultZoomLevel <= 95){ defaultZoomLevel+=5; }
            for (int i = 0; i < 5; i++) {
                this->getCurrentPage()->setZoomLevelRequest(defaultZoomLevel);
            }

        });
        addAction(enlargViewAction);

        auto shrinkViewAction = new QAction(this);
        shrinkViewAction->setShortcut(QKeySequence::ZoomOut);
        connect(shrinkViewAction, &QAction::triggered, this, [=]() {
            int defaultZoomLevel = this->currentViewZoomLevel();
            if (defaultZoomLevel >= 5) {
                defaultZoomLevel-=5;
            } else {
                defaultZoomLevel = 0;
            }
            this->getCurrentPage()->setZoomLevelRequest(defaultZoomLevel);
        });
        addAction(shrinkViewAction);

        auto quitAllAction = new QAction(this);
        quitAllAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
        connect(quitAllAction, &QAction::triggered, this, [=]() {
            QProcess p(0);
            p.start("/usr/bin/explor", QStringList()<<"-q");
            p.waitForStarted();
            p.waitForFinished();
        });
        addAction(quitAllAction);

        auto refreshAction = new QAction(this);
        refreshAction->setShortcut(Qt::Key_F5);
        connect(refreshAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshAction);

        //select all files in view, fix bug#115442
        auto *selectAllAction = new QAction(this);
        selectAllAction->setShortcut(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, this, [=]() {
            if (this->getCurrentPage()->getView())
            {
                /// note: 通过getAllFileUris设置的全选效率过低，如果增加接口则会导致二进制兼容性问题
                /// 所以这里使用现有的反选接口实现高效的全选，这个方法在mainwindow中也有用到
                //auto allFiles = this->getCurrentPage()->getView()->getAllFileUris();
                //this->getCurrentPage()->getView()->setSelections(allFiles);
                this->getCurrentPage()->getView()->setSelections(QStringList());
                this->getCurrentPage()->getView()->invertSelections();
            }
        });
        addAction(selectAllAction);

        //file operations
        auto *copyAction = new QAction(this);
        copyAction->setShortcut(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, [=]() {
            bool is_recent = false;
            if (!this->getCurrentSelections().isEmpty())
            {
//                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
//                    return ;
//                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    is_recent = true;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }
            }
            else
                return;

            QStringList selections;
            if (is_recent)
            {
                for(auto uri:this->getCurrentSelections())
                {
                    uri = Peony::FileUtils::getTargetUri(uri);
                    selections << uri;
                }
            }
            else{
                selections = this->getCurrentSelections();
            }

            Peony::ClipboardUtils::setClipboardFiles(selections, false);
        });
        addAction(copyAction);

        auto *pasteAction = new QAction(this);
        pasteAction->setShortcut(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("trash://") || currentUri.startsWith("recent://")
                || currentUri.startsWith("computer://") || currentUri.startsWith("favorite://")
                || currentUri.startsWith("search://") || currentUri == "filesafe:///"
                || currentUri.startsWith("burn://"))
            {
                /* Add hint information,link to bug#107640. */
                QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
                return;
            }

            //fix bug#183268, not allow paste in mtp, gphoto2 path or can not write path
            auto info = Peony::FileInfo::fromUri(currentUri);
            //comment to fix bug#191108, huawei phone can paste file success
            if (!info->canWrite() /*|| currentUri.startsWith("mtp://")
                || currentUri.startsWith("gphoto2://")*/) {
                return;
            }

            Peony::ClipboardUtils::getInstance()->updateClipboardManually();
            if (Peony::ClipboardUtils::isClipboardHasFiles()) {
                //FIXME: how about duplicated copy?
                //FIXME: how to deal with a failed move?
                auto op = Peony::ClipboardUtils::pasteClipboardFiles(this->getCurrentUri());
                if (op) {
                    connect(op, &Peony::FileOperation::operationFinished, this, [=](){
                        auto opInfo = op->getOperationInfo();
                        auto targetUirs = opInfo->dests();
                        setCurrentSelectionUris(targetUirs);
                    }, Qt::BlockingQueuedConnection);
                }
                else{
                    //fix paste file in old path not update issue, link to bug#71627
                    this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(pasteAction);

        auto *cutAction = new QAction(this);
        cutAction->setShortcut(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, [=]() {
            if (!this->getCurrentSelections().isEmpty()) {
//                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
//                    return ;
//                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    return ;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }

                auto currentUri = getCurrentUri();
                if (currentUri.startsWith("search://"))
                    return;

                auto info = Peony::FileInfo::fromUri(currentUri);
                if (!info->canWrite()) {
                    return;
                }

                QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
                QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                if (! this->getCurrentSelections().contains(desktopUri) && ! this->getCurrentSelections().contains(homeUri))
                {
                   Peony::ClipboardUtils::setClipboardFiles(this->getCurrentSelections(), true);
                   this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(cutAction);

        m_shortcuts_set = true;
    }
}

void MainWindow::updateTabPageTitle()
{
    m_tab->updateTabPageTitle();
    //FIXME: replace BLOCKING api in ui thread.
    auto curUri = getCurrentUri();
    auto show = Peony::FileUtils::getFileDisplayName(curUri);
    QString title = show;// + "-" + tr("File Manager");
    if (curUri.startsWith("search:///"))
        title = tr("Search");
    if (title.length() <= 0)
        title = tr("File Manager");
    //qDebug() << "updateTabPageTitle:" <<title;
    setWindowTitle(title);
}

void MainWindow::createFolderOperation()
{
//    Peony::CreateTemplateOperation op(getCurrentUri(), Peony::CreateTemplateOperation::EmptyFolder, tr("New Folder"));
//    Peony::FileOperationErrorDialogConflict dlg;
//    connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogConflict::handle);
//    op.run();
//    auto targetUri = op.target();

    auto op = Peony::FileOperationUtils::create(getCurrentUri(), tr("New Folder"), Peony::CreateTemplateOperation::EmptyFolder);
    connect(op, &Peony::FileOperation::operationFinished, this, [=](){
        if (op->hasError())
            return;
        auto opInfo = op->getOperationInfo();
        //auto targetUri = opInfo->target();
        this->getCurrentPage()->getView()->clearIndexWidget();
        //set a short time delay, fix bug#86070, select two folders
        QTimer::singleShot(10, this, [=](){
            this->editUri(opInfo->target());
        });
    }, Qt::BlockingQueuedConnection);

//    QTimer::singleShot(500, this, [=]() {
//        this->getCurrentPage()->getView()->scrollToSelection(targetUri);
//        this->editUri(targetUri);
//    });
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        auto uri = Peony::FileUtils::getParentUri(getCurrentUri());
        //qDebug() << "goUp Action" << getCurrentUri() << uri;
        if (uri.isNull())
            return;
        m_tab->goToUri(uri, true, true);
    }

    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        auto selections = this->getCurrentSelections();
        //if select only one item, let view to process
        if (selections.count() > 1)
        {
            QString currentUri = this->getCurrentUri();
            if(currentUri == "filesafe:///") {
                return ;
            }

            QStringList files;
            QStringList dirs;
            for (auto uri : selections) {
                auto info = Peony::FileInfo::fromUri(uri);
                if (info->isDir() || info->isVolume()) {
                    dirs<<uri;
                } else {
                    files<<uri;
                }
            }
            for (auto uri : dirs) {
                m_tab->addPage(uri);
            }

            QMap<QString, QStringList> fileMap;
            for (auto uri : files) {
                QString defaultAppName = Peony::FileLaunchManager::getDefaultAction(uri)->getAppInfoName();
                QStringList list;
                if (fileMap.contains(defaultAppName)) {
                    list = fileMap[defaultAppName];
                    list << uri;
                    fileMap.insert(defaultAppName, list);
                } else {
                    list << uri;
                    fileMap.insert(defaultAppName, list);
                }
            }
            if(!fileMap.empty()) {
                QMap<QString, QStringList>::iterator iter = fileMap.begin();
                while (iter != fileMap.end())
                {
                    Peony::FileLaunchManager::openAsync(iter.value());
                    iter++;
                }
            }
        }
    }

    return QMainWindow::keyPressEvent(e);
}

const QString MainWindow::getCurrentUri()
{
    return m_tab->getCurrentUri();
}

const QStringList MainWindow::getCurrentSelections()
{
    return m_tab->getCurrentSelections();
}

const QStringList MainWindow::getCurrentAllFileUris()
{
    return m_tab->getAllFileUris();
}

Qt::SortOrder MainWindow::getCurrentSortOrder()
{
    return m_tab->getSortOrder();
}

int MainWindow::getCurrentSortColumn()
{
    return m_tab->getSortType();
}

bool MainWindow::getWindowShowHidden()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): false;
        } else {
            qDebug()<<"can not get file meta info"<<uri;
            return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
        }
    }
}

bool MainWindow::getWindowUseDefaultNameSortOrder()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): true;
        } else {
            qDebug()<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
        }
    }
}

bool MainWindow::getWindowSortFolderFirst()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): true;
        } else {
            qDebug()<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
        }
    }
}

int MainWindow::currentViewZoomLevel()
{
    if (getCurrentPage()) {
        if (auto view = getCurrentPage()->getView()) {
            return view->currentZoomLevel();
        }
    }

    int defaultZoomLevel = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ZOOM_LEVEL).toInt();

    if (defaultZoomLevel >= 0 && defaultZoomLevel <= 100) {
        return defaultZoomLevel;
    }

    return m_tab->m_status_bar->m_slider->value();
}

bool MainWindow::currentViewSupportZoom()
{
    if (getCurrentPage()) {
        if (auto view = getCurrentPage()->getView()) {
            return view->supportZoom();
        }
    }
    return m_tab->m_status_bar->m_slider->isEnabled();
}

void MainWindow::maximizeOrRestore()
{
    if (!this->isMaximized()) {
        this->showMaximized();
    } else {
        this->showNormal();
    }
    //maybe not need update icons? comment to try fix bug#77966
    //m_header_bar->updateIcons();
    m_header_bar->updateMaximizeState();
}

void MainWindow::syncControlsLocation(const QString &uri)
{
    m_tab->goToUri(uri, false, false);
}

void MainWindow::updateHeaderBar()
{
    m_header_bar->setLocation(getCurrentUri());
    m_header_bar->updateIcons();
    //fix bug#82685
    m_header_bar->updateSortTypeEnable();
    //fix bug#66336, 83711
    m_header_bar->updateViewTypeEnable();
    //m_status_bar->update();
}

void MainWindow::updateWindowIcon()
{
   auto currentUri = getCurrentUri();
   if (currentUri.startsWith("trash://"))
   {
       QIcon icon = QIcon::fromTheme("user-trash");
       qApp->setWindowIcon(icon);
   }
   else if (currentUri.startsWith("computer://"))
   {
       QIcon icon = QIcon::fromTheme("computer");
       qApp->setWindowIcon(icon);
   }
   else
   {
       QIcon icon = QIcon::fromTheme("system-file-manager");
       qApp->setWindowIcon(icon);
   }
}

void MainWindow::goToUri(const QString &uri, bool addHistory, bool force)
{
    QUrl url(uri);
    auto realUri = uri;
    if (uri == "computer:///lingmo-data-volume") {
        realUri = "file:///data";
    }
    //process open symbolic link
    auto info = Peony::FileInfo::fromUri(uri);
    if (info->isSymbolLink() && info->symlinkTarget().length() >0 &&
    (uri.startsWith("file://") || uri.startsWith("favorite://")))
        realUri = "file://" + info->symlinkTarget();

    //try to fix bug#174666, part phone mtp mode access wrong issue
    if (url.scheme().isEmpty() && ! uri.startsWith("mtp://") && ! uri.startsWith("gphoto2://")) {
        qDebug() << "transform special uri:"<<uri;
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

    //Fix bug#132638, special # character use in symbolic link open fail issue
    if (realUri.contains("\#") && ! realUri.startsWith("filesafe:///"))
        realUri = Peony::FileUtils::urlEncode(realUri);

    //if in search mode and key is not null, need quit search mode, bug#93528
    //清空搜索关键字时，不应该退出搜索状态，其他情况下，跳转非搜索路径，需要退出搜索
    if (! m_is_clear_serach && m_is_search  && ! uri.startsWith("search://"))
    {
        m_is_search = false;
        m_header_bar->searchButtonClicked();
    }

    if (getCurrentUri() == realUri) {
        if (!force) {
            refresh();
            return;
        }
    }

    locationChangeStart();
    m_tab->goToUri(realUri, addHistory, force);
    m_header_bar->setLocation(uri);

    m_label_box->clearSelection();
    Q_EMIT m_label_box->leftClickOnBlank();
}

void MainWindow::updateSearch(const QString &uri, const QString &key, bool updateKey)
{
    qDebug() << "updateSearch:" <<uri <<key <<updateKey;
    bool needUpdate = false;
    //coment 9X0 changes, fix bug#70916
    //m_tab->enableSearchBar(key.length() != 0);
    if (m_last_search_path == "" || ! Peony::FileUtils::isSamePath(uri, m_last_search_path))
    {
       //qDebug() << "updateSearch:" <<uri;
       m_last_search_path = uri;
       needUpdate = true;
    }

    if (updateKey)
    {
        needUpdate = true;
        m_last_key = key;
    }

    if (needUpdate)
    {
        //qDebug() << "updateSearch needUpdate:" <<m_last_key<<m_last_search_path;
        forceStopLoading();
        if (m_last_key == ""){
            m_is_clear_serach = true;
            goToUri(m_last_search_path, true);
            m_is_clear_serach = false;
            m_tab->m_status_bar->updateSearchProgress(false);
            m_searching = false;
        }
        else
        {
            bool isSearchEngine = true;
            const QByteArray id(LINGMO_SEARCH_SCHEMAS);
            if (QGSettings::isSchemaInstalled(id)) {
                QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
                if (!searchSettings || !searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
                    isSearchEngine = false;
                }
            } else {
                isSearchEngine = false;
            }

            if (!m_last_search_path.startsWith("file:///") && !m_last_search_path.startsWith("computer:///")) {
                isSearchEngine = false;
            }

            auto targetUri = Peony::SearchVFSUriParser::parseSearchKey(m_last_search_path,
                                                         m_last_key, true, false, "", true);
            targetUri = Peony::SearchVFSUriParser::addSearchKey(targetUri, isSearchEngine);
            //qDebug() << "updateSearch targetUri:" <<targetUri;
            goToUri(targetUri, true);
            m_tab->m_status_bar->updateSearchProgress(true);
            m_searching = true;
        }
    }
}

void MainWindow::addNewTabs(const QStringList &uris)
{
    //fix search path add new tab,page title show abnormal issue
    if (uris.count() == 1)
    {
        m_tab->addPage(uris.first(), true);
        return;
    }

    for (auto uri : uris) {
        m_tab->addPage(uri, false);
    }
}

void MainWindow::beginSwitchView(const QString &viewId)
{
    //not allow change to other view when in computer, link to bug#83711
    if (getCurrentUri() == "computer:///")
        return;

    auto selection = getCurrentSelections();
//    int sortType = getCurrentSortColumn();
//    Qt::SortOrder sortOrder = getCurrentSortOrder();
    m_tab->switchViewType(viewId);
    // save zoom level
    Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ZOOM_LEVEL, currentViewZoomLevel());
    m_tab->setCurrentSelections(selection);
    bool supportZoom = m_tab->currentPage()->getView()->supportZoom();
    m_tab->m_status_bar->m_slider->setEnabled(supportZoom);
    m_tab->m_status_bar->m_slider->setVisible(supportZoom);
    //fix slider value not update issue
    m_tab->m_status_bar->m_slider->setValue(currentViewZoomLevel());
}



void MainWindow::refresh()
{
    locationChangeStart();
    m_tab->refresh();
    //fix refresh clear copy files issue, link to bug#109247
    if (Peony::ClipboardUtils::isPeonyFilesBeCut())
        Peony::ClipboardUtils::clearClipboard();/* Refresh clear cut status */
    //goToUri(getCurrentUri(), false, true);
}

void MainWindow::setLabelNameFilter(QString name)
{
    if (!getCurrentPage()) {
        return;
    }
    //update filter flag
    if (name == "")
        m_filter_working = false;
    else
        m_filter_working = true;
    m_header_bar->updateHeaderState();
    getCurrentPage()->setFilterLabelConditions(name);
}

void MainWindow::setShowHidden(bool showHidden)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setShowHidden(showHidden);
    //显示隐藏文件，更新项目个数
    Q_EMIT m_tab->updateItemsNum();
}

void MainWindow::setShowFileExtensions(bool checked)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setShowFileExtensions(checked);
}

void MainWindow::setUseDefaultNameSortOrder(bool use)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setUseDefaultNameSortOrder(use);
}

void MainWindow::setSortFolderFirst(bool set)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setSortFolderFirst(set);
}

void MainWindow::forceStopLoading()
{
    m_tab->stopLoading();

    //Key_escape also use as cancel
    if (Peony::ClipboardUtils::isClipboardHasFiles())
    {
        Peony::ClipboardUtils::clearClipboard();
        this->getCurrentPage()->getView()->repaintView();
    }
}

void MainWindow::setCurrentSelectionUris(const QStringList &uris)
{
    m_tab->setCurrentSelections(uris);
    //move scrollToSelection to m_tab to try fix new unzip file show two same icon issue
    //Fix me, unknow caused reason
//    if (uris.isEmpty())
//        return;
//    getCurrentPage()->getView()->scrollToSelection(uris.first());
}

void MainWindow::setCurrentSortOrder(Qt::SortOrder order)
{
    m_tab->setSortOrder(order);
}

void MainWindow::setCurrentSortColumn(int sortColumn)
{
    m_tab->setSortType(sortColumn);
}

void MainWindow::editUri(const QString &uri)
{
    m_tab->editUri(uri);
}

void MainWindow::editUris(const QStringList &uris)
{
    m_tab->editUris(uris);
}

void MainWindow::setCurrentViewZoomLevel(int zoomLevel)
{
    if (currentViewSupportZoom())
        m_tab->m_status_bar->m_slider->setValue(zoomLevel);
}

QString MainWindow::getLastSearchKey()
{
    return m_last_key;
}


void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    //may not need update? comment to try fix bug#77966
    //m_header_bar->updateMaximizeState();
    update();

    if (!isMaximized()) {
        // set save window size flag
        last_resize_window = this;
    }
}

/*!
 * \note
 * The window has a noticeable tearing effect due to the drawing of shadow effects.
 * I should consider do not painting a shadow when resizing.
 */
void MainWindow::paintEvent(QPaintEvent *e)
{
    QColor color = qApp->palette().color(QPalette::Window);
    QColor colorBase = color;

    auto sidebarOpacity = Peony::GlobalSettings::getInstance()->getValue(SIDEBAR_BG_OPACITY).toInt();

    if (m_is_blur_window) {
        colorBase.setAlphaF(sidebarOpacity/100.0);
    }

    QPainter p(this);
    p.fillRect(this->rect(), color);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(sideBarRect(), colorBase);

    QMainWindow::paintEvent(e);
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    //qDebug()<<"mouse pressed"<<e;
    QMainWindow::mousePressEvent(e);
    if (e->button() == Qt::LeftButton && !e->isAccepted()) {
        m_is_draging = true;
        m_offset = mapFromGlobal(QCursor::pos());
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    //NOTE: when starting a X11 window move, the mouse move event
    //will unreachable when draging, and after draging we could not
    //get the release event correctly.
    //qDebug()<<"mouse move";
    QMainWindow::mouseMoveEvent(e);
//    if (!m_is_draging)
//        return;

//    qreal  dpiRatio = qApp->devicePixelRatio();
//    if (QX11Info::isPlatformX11()) {
//        Display *display = QX11Info::display();
//        Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
//        XEvent xEvent;
//        const auto pos = QCursor::pos();

//        memset(&xEvent, 0, sizeof(XEvent));
//        xEvent.xclient.type = ClientMessage;
//        xEvent.xclient.message_type = netMoveResize;
//        xEvent.xclient.display = display;
//        xEvent.xclient.window = this->winId();
//        xEvent.xclient.format = 32;
//        xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
//        xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
//        xEvent.xclient.data.l[2] = 8;
//        xEvent.xclient.data.l[3] = Button1;
//        xEvent.xclient.data.l[4] = 0;

//        XUngrabPointer(display, CurrentTime);
//        XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
//                   False, SubstructureNotifyMask | SubstructureRedirectMask,
//                   &xEvent);
//        //XFlush(display);

//        XEvent xevent;
//        memset(&xevent, 0, sizeof(XEvent));

//        xevent.type = ButtonRelease;
//        xevent.xbutton.button = Button1;
//        xevent.xbutton.window = this->winId();
//        xevent.xbutton.x = e->pos().x() * dpiRatio;
//        xevent.xbutton.y = e->pos().y() * dpiRatio;
//        xevent.xbutton.x_root = pos.x() * dpiRatio;
//        xevent.xbutton.y_root = pos.y() * dpiRatio;
//        xevent.xbutton.display = display;

//        XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xevent);
//        XFlush(display);

//        if (e->source() == Qt::MouseEventSynthesizedByQt) {
//            if (!this->mouseGrabber()) {
//                this->grabMouse();
//                this->releaseMouse();
//            }
//        }

//        m_is_draging = false;
//    } else {
//        this->move((QCursor::pos() - m_offset) * dpiRatio);
//    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    /*!
     * \bug
     * release event sometimes "disappear" when we request
     * X11 window manager for movement.
     */
    QMainWindow::mouseReleaseEvent(e);
    //qDebug()<<"mouse released";
    m_is_draging = false;
}

void MainWindow::validBorder()
{
    //deprecated
}
#include "file-utils.h"
void MainWindow::initUI(const QString &uri)
{
    auto size = sizeHint();
    resize(size);
    m_searching = false;
    connect(this, &MainWindow::locationChangeStart, this, [=]() {
        //comment to fix bug 33527
        //m_side_bar->blockSignals(true);
        m_header_bar->blockSignals(true);
        QCursor c;
        c.setShape(Qt::BusyCursor);
        this->setCursor(c);
        m_tab->setCursor(c);
        m_side_bar->setCursor(c);
        //m_status_bar->update();
    });

    connect(this, &MainWindow::locationChangeEnd, this, [=]() {
        //comment to fix bug 33527
        //m_side_bar->blockSignals(false);
        m_header_bar->blockSignals(false);
        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        m_tab->setCursor(c);
        m_side_bar->setCursor(c);
        updateHeaderBar();
        //function for LINGMO3.1, update window icon
        //updateWindowIcon();
        //m_status_bar->update();

        if (m_searching) {
            m_tab->m_status_bar->updateSearchProgress(false);
            Q_EMIT m_header_bar->updateSearchProgress(false);
            m_searching = false;
        }
        setShortCuts();
    });

    //HeaderBar
    auto headerBar = new HeaderBar(this);
    m_header_bar = headerBar;
    auto headerBarContainer = new HeaderBarContainer(this);
    headerBarContainer->addHeaderBar(headerBar);
    addToolBar(headerBarContainer);
    //m_header_bar->setVisible(false);

    connect(m_header_bar, &HeaderBar::updateLocationRequest, this, &MainWindow::goToUri);
    connect(m_header_bar, &HeaderBar::viewTypeChangeRequest, this, &MainWindow::beginSwitchView);
    connect(m_header_bar, &HeaderBar::updateZoomLevelHintRequest, this, [=](int zoomLevelHint) {
        if (zoomLevelHint >= 0) {
            m_tab->m_status_bar->m_slider->setEnabled(true);
            m_tab->m_status_bar->m_slider->setValue(zoomLevelHint);
        } else {
            m_tab->m_status_bar->m_slider->setEnabled(false);
        }
    });

    //SideBar
    QDockWidget *sidebarContainer = new QDockWidget(this);

    sidebarContainer->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto palette = sidebarContainer->palette();
    palette.setColor(QPalette::Window, Qt::transparent);
    sidebarContainer->setPalette(palette);
//    sidebarContainer->setStyleSheet("{"
//                                    "background-color: transparent;"
//                                    "border: 0px solid transparent"
//                                    "}");
    sidebarContainer->setTitleBarWidget(new QWidget(this));
    sidebarContainer->titleBarWidget()->setFixedHeight(0);
    sidebarContainer->setAttribute(Qt::WA_TranslucentBackground);
    sidebarContainer->setContentsMargins(0, 0, 0, 0);

    NavigationSideBar *sidebar = new NavigationSideBar(this);
    m_side_bar = sidebar;

    auto navigationSidebarContainer = new NavigationSideBarContainer(this);
    navigationSidebarContainer->addSideBar(m_side_bar);

    m_transparent_area_widget = navigationSidebarContainer;

    connect(m_side_bar, &NavigationSideBar::updateWindowLocationRequest, this, &MainWindow::goToUri);

    auto labelDialog = new FileLabelBox(this);
    labelDialog->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    labelDialog->hide();
    m_label_box = labelDialog;

    connect(labelDialog->selectionModel(), &QItemSelectionModel::selectionChanged, [=]()
    {
        auto selected = labelDialog->selectionModel()->selectedIndexes();
        //qDebug() << "FileLabelBox selectionChanged:" <<selected.count();
        if (selected.count() > 0)
        {
            auto name = selected.first().data().toString();
            setLabelNameFilter(name);
        }
    });
    //when clicked in blank, currentChanged may not triggered
    connect(labelDialog, &FileLabelBox::leftClickOnBlank, [=]()
    {
        setLabelNameFilter("");
    });



    sidebarContainer->setWidget(navigationSidebarContainer);
    addDockWidget(Qt::LeftDockWidgetArea, sidebarContainer);

//    m_status_bar = new Peony::StatusBar(this, this);
//    setStatusBar(m_status_bar);

    auto views = new TabWidget;
    m_tab = views;
    if (uri.isNull()) {
        auto home = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        m_tab->addPage(home, true);
    } else {
        m_tab->addPage(uri, true);
        //m_header_bar->setLocation(uri);
    }
    QTimer::singleShot(1, this, [=]() {
        // FIXME:
        // it is strange that if we set size hint by qsettings,
        // the tab bar will "shrink" until we update geometry
        // after a while (may resize window or move splitter).
        // we should find out the reason and remove this dirty
        // code.
        m_tab->updateTabBarGeometry();
        m_tab->updateStatusBarGeometry();
    });
//    auto view = getCurrentPage()->getView();
//    Q_EMIT Peony::DirectoryViewHelper::globalInstance()->setLabelAlignment(0);
    Peony::GlobalSettings::getInstance()->setValue(LABLE_ALIGNMENT, 0);
    connect(views->tabBar(), &QTabBar::tabBarDoubleClicked, this, [=](int index) {
        if (index == -1)
            maximizeOrRestore();
    });
    connect(views, &TabWidget::closeWindowRequest, this, &QWidget::close);
    //connect(m_header_bar, &HeaderBar::updateSearchRequest, this, &MainWindow::updateSearchStatus);
    connect(m_header_bar, &HeaderBar::updateSearch, this, &MainWindow::updateSearch);

    X11WindowManager *tabBarHandler = X11WindowManager::getInstance();
    tabBarHandler->registerWidget(views->tabBar());
    tabBarHandler->registerWidget(m_header_bar);

    auto paneAndViews = new FloatPaneWidget(views, labelDialog, this);
    connect(sidebar, &NavigationSideBar::labelButtonClicked, this, [=](bool checked)
    {
        bool visible = checked;
        //quit label filter, clear conditions
        if (visible)
        {
           setLabelNameFilter("");
           m_label_box->clearSelection();
        } else {
            setLabelNameFilter("");
        }
        paneAndViews->setFloatWidgetVisible(visible);
    });
    setCentralWidget(paneAndViews);

//    // check slider zoom level
//    if (currentViewSupportZoom())
//        setCurrentViewZoomLevel(currentViewZoomLevel());

    //bind signals
    connect(m_tab, &TabWidget::searchRecursiveChanged, headerBar, &HeaderBar::updateSearchRecursive);
    connect(m_tab, &TabWidget::closeSearch, headerBar, &HeaderBar::closeSearch);
    connect(m_tab, &TabWidget::clearTrash, this, &MainWindow::cleanTrash);
    connect(this, &MainWindow::trashcleaned, m_tab, [=](){
        m_tab->updateTabPageTitle();
    });
    connect(this, &MainWindow::trashcleaned, headerBar, &HeaderBar::clearTrash);
    connect(m_tab, &TabWidget::recoverFromTrash, this, &MainWindow::recoverFromTrash);
    connect(m_tab, &TabWidget::updateWindowLocationRequest, this, &MainWindow::goToUri);
    connect(m_tab, &TabWidget::updateSearch, this, &MainWindow::updateSearch);
    connect(m_tab, &TabWidget::activePageLocationChanged, this, &MainWindow::locationChangeEnd);
    connect(m_tab, &TabWidget::activePageViewTypeChanged, this, &MainWindow::updateHeaderBar);
    connect(m_tab, &TabWidget::activePageChanged, this, &MainWindow::updateHeaderBar);
    connect(m_tab, &TabWidget::activePageChanged, this, [=](){
        // check slider zoom level
        setCurrentViewZoomLevel(currentViewZoomLevel());
    });

    connect(m_tab, &TabWidget::signal_itemAdded, this, [=](const QString& uri){
        /* 新建文件/文件夹，可编辑文件名，copy时不能编辑 */
        if(this->m_uris_to_edit.isEmpty())
            return;
        QString editUri = Peony::FileUtils::urlDecode(this->m_uris_to_edit.first());
        QString infoUri = Peony::FileUtils::urlDecode(uri);
        if (editUri == infoUri ) {
            this->getCurrentPage()->getView()->scrollToSelection(uri);
            this->editUri(uri);
        }
        this->m_uris_to_edit.clear();
    });

    connect(m_tab, &TabWidget::menuRequest, this, [=](const QPoint &pos) {
        //fix bug#162775, show mutiple menu issue
        if (! m_is_show_menu){
            m_is_show_menu = true;
            Peony::DirectoryViewMenu menu(this, this);
            menu.exec(pos);
            m_uris_to_edit = menu.urisToEdit();
            m_is_show_menu = false;
        }
    });

    connect(m_tab, &TabWidget::updateWindowSelectionRequest, this, [=](const QStringList &uris){
        setCurrentSelectionUris(uris);
    });
//    connect(m_tab, &TabWidget::currentSelectionChanged, this, [=](){
//        m_status_bar->update();
//    });

    addFocusWidgetToFocusList(m_side_bar);
    addFocusWidgetToFocusList(m_tab);

    setTabOrder(nullptr, this);

    QWidget *oldWidget = this;
    for (auto widget : m_focus_list) {
        setTabOrder(oldWidget, widget);
        oldWidget = widget;
        if (m_focus_list.last() == widget) {
            setTabOrder(widget, m_focus_list.first());
        }
    }
}

void MainWindow::updateSearchStatus(bool showSearch)
{
    m_tab->updateSearchBar(showSearch);
    m_header_bar->setSearchMode(showSearch);
    m_is_search = showSearch;
}

void MainWindow::cleanTrash()
{
    auto uris = getCurrentAllFileUris();
    if (uris.count() >0)
    {
        auto removeop = Peony::FileOperationUtils::clearRecycleBinWithDialog(uris, this);
        qApp->setProperty("clearTrash",true);
        if(removeop){
            removeop->connect(removeop,&Peony::FileDeleteOperation::operationFinished,this,[=](){
//                Peony::SoundEffect::getInstance()->recycleBinClearMusic();
                Q_EMIT trashcleaned();
            });
        }
    }
    else
    {     /* 由于QMessageBox的setParent还不支持，暂先注释处理，link to bug#22692 【回收站】清空时，其他工作区的文件管理器会转到当前工作区 */
//        QMessageBox::information(nullptr, tr("Tips info"),
//                                 tr("Trash has no file need to be cleaned."));

    }
}

void MainWindow::recoverFromTrash()
{
    auto m_selections = getCurrentSelections();
    if (m_selections.isEmpty())
        m_selections = getCurrentAllFileUris();
    if (m_selections.count() == 1) {
        Peony::FileOperationUtils::restore(m_selections.first());
    } else {
        Peony::FileOperationUtils::restore(m_selections);
    }
}

QRect MainWindow::sideBarRect()
{
    auto pos = m_transparent_area_widget->mapTo(this, QPoint());
    return QRect(pos, m_transparent_area_widget->size());
}

void MainWindow::startMonitorThumbnailForbidStatus()
{
    m_do_not_thumbnail = Peony::GlobalSettings::getInstance()->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool();

    m_thumbnail_watcher = new QFileSystemWatcher(this);
    QString explorSettingFile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                        + "/.config/org.lingmo/explor-qt-preferences.conf";
    m_thumbnail_watcher->addPath(explorSettingFile);

    connect(m_thumbnail_watcher, &QFileSystemWatcher::fileChanged, [=](const QString &uri){
        auto settings = Peony::GlobalSettings::getInstance();
        if (m_do_not_thumbnail != settings->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool()) {
            m_do_not_thumbnail = settings->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool();
            if (true == m_do_not_thumbnail) {
                Peony::ThumbnailManager::getInstance()->clearThumbnail();
            }
            refresh();
        }

        //qDebug()<<"explorSettingFile:"<<explorSettingFile;
        m_thumbnail_watcher->addPath(uri);
    });

}

void MainWindow::addFocusWidgetToFocusList(QWidget *widget)
{
    m_focus_list<<widget;
}

QWidgetList MainWindow::focusWidgetsList()
{
    return m_focus_list;
}

const QList<std::shared_ptr<Peony::FileInfo>> MainWindow::getCurrentSelectionFileInfos()
{
    const QStringList uris = getCurrentSelections();
    QList<std::shared_ptr<Peony::FileInfo>> infos;
    for(auto uri : uris) {
        auto info = Peony::FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}
