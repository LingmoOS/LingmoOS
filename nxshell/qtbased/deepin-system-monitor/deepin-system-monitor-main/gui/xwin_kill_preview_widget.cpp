// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xwin_kill_preview_widget.h"

#include "application.h"
#include "xwin_kill_preview_background_widget.h"
#include "wm/wm_info.h"
#include "main_window.h"
#include "ui_common.h"
#include "common/common.h"
#include "ddlog.h"
#ifdef USE_DEEPIN_WAYLAND
#    include "3rdparty/displayjack/wayland_client.h"
#endif

#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QScreen>
#include <QWindow>
#include <QRect>
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QRegion>
#include <QTimer>

#include <unistd.h>
#include <string>

using namespace std;
using namespace core::wm;
using namespace common::init;
using namespace DDLog;

typedef int (*InitDtkWmDisplayPtr)();
typedef void (*DestoryDtkWmDisplayPtr)();
typedef int (*GetAllWindowStatesListPtr)(WindowState **states);

// displayjack库接口
static InitDtkWmDisplayPtr InitDtkWmDisplay = nullptr;
static DestoryDtkWmDisplayPtr DestoryDtkWmDisplay = nullptr;
static GetAllWindowStatesListPtr GetAllWindowStatesList = nullptr;

// constructor
XWinKillPreviewWidget::XWinKillPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    // new window manager instance
    m_wminfo = new WMInfo();
//不再使用CMakeList开关宏的方式，改用全局变量运行时控制
//WaylandCentered定义在common/common.h中，在main函数开头进行初始化判断
#ifdef USE_DEEPIN_WAYLAND
    if (WaylandCentered) {
        m_connectionThread = new QThread(this);
        m_connectionThreadObject = new ConnectionThread();

        QLibrary library("libdtkwmjack.so");
        InitDtkWmDisplay = reinterpret_cast<int (*)()>(library.resolve("InitDtkWmDisplay"));
        DestoryDtkWmDisplay = reinterpret_cast<void (*)()>(library.resolve("DestoryDtkWmDisplay"));
        GetAllWindowStatesList = reinterpret_cast<int (*)(WindowState **)>(library.resolve("GetAllWindowStatesList"));

        if (InitDtkWmDisplay)
            InitDtkWmDisplay();
    }
#endif   // USE_DEEPIN_WAYLAND

    // init ui components & connections
    initUI();
    initConnections();

    // resize preview widget to 1x1 in case any obvious black flash widgets occassionly shown on screen
    resize(1, 1);
    // move preview widget way further away so it's impossible to show on screen
    move(-65536, -65536);
    show();
}

// destructor
XWinKillPreviewWidget::~XWinKillPreviewWidget()
{
    // restore window state
    auto *mw = gApp->mainWindow();
    mw->setWindowState((mw->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    // release any owned mouse & keyboard
    setMouseTracking(false);
    releaseMouse();
    releaseKeyboard();
    delete m_wminfo;

#ifdef USE_DEEPIN_WAYLAND
    if (WaylandCentered) {
        m_connectionThread->quit();
        m_connectionThread->wait();
        m_connectionThreadObject->deleteLater();

        if (DestoryDtkWmDisplay)
            DestoryDtkWmDisplay();
    }
#endif   // USE_DEEPIN_WAYLAND
}

// mouse press event
void XWinKillPreviewWidget::mousePressEvent(QMouseEvent *event)
{
    // only accept left mouse button click event
    if (event->button() != Qt::LeftButton) {
        return;
    }
    // get the list of windows under cursor in stacked order when mouse pressed
    auto pos = QCursor::pos();
#ifdef USE_DEEPIN_WAYLAND
    if (WaylandCentered) {
        double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();   // 获得当前的缩放比例
        QRect screenRect;
        for (auto screen : QApplication::screens()) {
            // screen geometry
            auto geom = screen->geometry();
            if (geom.contains(pos)) {
                screenRect = geom;
                break;
            }
        }
        for (QVector<ClientManagement::WindowState>::iterator it = m_windowStates.end() - 1;
             it != m_windowStates.begin(); --it) {
            // if the window is created by ourself, then ignore it
            if (getpid() == it->pid || QString::fromStdString(it->resourceName) == "dde-desktop" || it->isMinimized
                || QString::fromStdString(it->resourceName) == "deepin-deepinid-client"
                || QString::fromStdString(it->resourceName) == "dde-dock")
                continue;

            // if such window exists, we emit window clicked signal to notify kill application performed action
            QRect rect(static_cast<int>(screenRect.x() + (it->geometry.x - screenRect.x()) / ratio),
                       static_cast<int>(screenRect.y() + (it->geometry.y - screenRect.y()) / ratio),
                       static_cast<int>(it->geometry.width / ratio), static_cast<int>(it->geometry.height / ratio));
            if (rect.contains(pos)) {
                // hide preview & background widgets first
                hide();
                for (auto &background : m_backgroundList) {
                    background->hide();
                }
                setMouseTracking(false);
                releaseMouse();
                releaseKeyboard();
                emit windowClicked(it->pid);
                close();
                break;
            }
        }
    }
#endif   // USE_DEEPIN_WAYLAND

    if (!WaylandCentered) {
        double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();   // 获得当前的缩放比例
        QRect screenRect;
        for (auto screen : QApplication::screens()) {
            // screen geometry
            auto geom = screen->geometry();
            if (geom.contains(pos)) {
                screenRect = geom;
                break;
            }
        }
        QPoint windowPos(static_cast<int>(screenRect.x() + (pos.x() - screenRect.x()) * ratio),
                         static_cast<int>(screenRect.y() + (pos.y() - screenRect.y()) * ratio));
        auto list = m_wminfo->selectWindow(windowPos);

        // fix cursor not update issue while moved to areas covered by intersected area of dock & normal windows
        if (m_wminfo->isCursorHoveringDocks(windowPos)) {
            return;
        }

        for (auto &select : list) {
            // if the window is created by ourself, then ignore it
            if (getpid() == select->pid)
                continue;

            // if such window exists, we emit window clicked signal to notify kill application performed action
            if (select->rect.contains(windowPos)) {
                // hide preview & background widgets first
                hide();
                for (auto &background : m_backgroundList) {
                    background->hide();
                }
                setMouseTracking(false);
                releaseMouse();
                releaseKeyboard();
                emit windowClicked(select->pid);
                close();
                break;
            }
        }
    }
}

// mouse move event handler
void XWinKillPreviewWidget::mouseMoveEvent(QMouseEvent *)
{
#ifdef USE_DEEPIN_WAYLAND
    if (WaylandCentered) {
        double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();   // 获得当前的缩放比例
        auto pos = QCursor::pos();
        QRect screenRect;
        for (auto screen : QApplication::screens()) {
            // screen geometry
            auto geom = screen->geometry();
            if (geom.contains(pos)) {
                screenRect = geom;
                break;
            }
        }
        // get the list of windows under cursor from cache in stacked order
        bool found { false };

        for (QVector<ClientManagement::WindowState>::iterator it = m_windowStates.end() - 1;
             it != m_windowStates.begin(); --it) {
            // if the window is created by ourself, then ignore it
            // wayland环境下增加桌面窗口和dock栏的屏蔽
            if (getpid() == it->pid || QString::fromStdString(it->resourceName) == "dde-desktop" || it->isMinimized
                || QString::fromStdString(it->resourceName) == "deepin-deepinid-client"
                || QString::fromStdString(it->resourceName) == "dde-dock")
                continue;
            auto selRect = QRect(static_cast<int>(screenRect.x() + (it->geometry.x - screenRect.x()) / ratio),
                                 static_cast<int>(screenRect.y() + (it->geometry.y - screenRect.y()) / ratio),
                                 static_cast<int>(it->geometry.width / ratio), static_cast<int>(it->geometry.height / ratio));
            if (selRect.contains(pos)) {
                found = true;

                // find all windows hovered above, if any clip out the intersected region

                QRegion region { selRect };
                //对于所选窗口上方存在堆叠的窗口情况，对所选窗口进行区域裁剪。
                for (QVector<ClientManagement::WindowState>::iterator iter = m_windowStates.end() - 1;
                     iter != it; --iter) {
                    if (QString::fromStdString(iter->resourceName) == "dde-desktop" || iter->isMinimized || QString::fromStdString(iter->resourceName) == "deepin-deepinid-client"
                        || QString::fromStdString(iter->resourceName) == "dde-dock" || getpid() == iter->pid)
                        continue;

                    else {
                        //上方的堆叠窗口区域
                        auto upRegion = QRect(static_cast<int>(screenRect.x() + (iter->geometry.x - screenRect.x()) / ratio),
                                              static_cast<int>(screenRect.y() + (iter->geometry.y - screenRect.y()) / ratio),
                                              static_cast<int>(iter->geometry.width / ratio), static_cast<int>(iter->geometry.height / ratio));

                        region = region.subtracted(upRegion);
                    }
                }

                // if current selected window is crossing screens, we need update each sub part on each screen
                for (auto &bg : m_backgroundList) {
                    if (bg->geometry().intersects(selRect)) {
                        auto area = region.intersected(bg->geometry());
                        bg->updateSelection(area);
                        emit cursorUpdated(m_killCursor);
                    } else {
                        // if current screen doesn't intersect with the selected window, we need clear any selection left before
                        bg->clearSelection();
                    }
                }
                break;
            }
        }
        // if no such window found, we need clear any selection left before, plus restore cursor to its default style (forbit style)
        if (!found) {
            for (auto &bg : m_backgroundList)
                bg->clearSelection();
            emit cursorUpdated(m_defaultCursor);
        }
    }
#endif   // USE_DEEPIN_WAYLAND
    if (!WaylandCentered) {
        double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();   // 获得当前的缩放比例
        auto pos = QCursor::pos();
        QRect screenRect;
        for (auto screen : QApplication::screens()) {
            // screen geometry
            auto geom = screen->geometry();
            if (geom.contains(pos)) {
                screenRect = geom;
                break;
            }
        }
        QPoint windowPos(static_cast<int>(screenRect.x() + (pos.x() - screenRect.x()) * ratio),
                         static_cast<int>(screenRect.y() + (pos.y() - screenRect.y()) * ratio));
        // get the list of windows under cursor from cache in stacked order
        auto list = m_wminfo->selectWindow(windowPos);
        bool found { false };

        // fix cursor not update issue while moved to areas covered by intersected area of dock & normal windows
        if (m_wminfo->isCursorHoveringDocks(windowPos)) {
            for (auto &bg : m_backgroundList)
                bg->clearSelection();
            emit cursorUpdated(m_defaultCursor);
            return;
        }
        for (auto &select : list) {
            // if the window is created by ourself, then ignore it
            if (getpid() == select->pid)
                continue;
            QRect selRect(static_cast<int>(screenRect.x() + (select->rect.x() - screenRect.x()) / ratio),
                          static_cast<int>(screenRect.y() + (select->rect.y() - screenRect.y()) / ratio),
                          static_cast<int>(select->rect.width() / ratio), static_cast<int>(select->rect.height() / ratio));
            if (selRect.contains(pos)) {
                found = true;

                // find all windows hovered above, if any clip out the intersected region
                auto hoveredBy = m_wminfo->getHoveredByWindowList(select->wid, select->rect);
                QRegion region { selRect };
                for (auto &hover : hoveredBy) {
                    QRect hoverrect(static_cast<int>(screenRect.x() + (hover->rect.x() - screenRect.x()) / ratio),
                                    static_cast<int>(screenRect.y() + (hover->rect.y() - screenRect.y()) / ratio),
                                    static_cast<int>(hover->rect.width() / ratio), static_cast<int>(hover->rect.height() / ratio));
                    region = region.subtracted(hoverrect);
                }

                // if current selected window is crossing screens, we need update each sub part on each screen
                for (auto &bg : m_backgroundList) {
                    if (bg->geometry().intersects(selRect)) {
                        auto area = region.intersected(bg->geometry());
                        bg->updateSelection(area);
                        emit cursorUpdated(m_killCursor);
                    } else {
                        // if current screen doesn't intersect with the selected window, we need clear any selection left before
                        bg->clearSelection();
                    }
                }
                break;
            }
        }
        // if no such window found, we need clear any selection left before, plus restore cursor to its default style (forbit style)
        if (!found) {
            for (auto &bg : m_backgroundList)
                bg->clearSelection();
            emit cursorUpdated(m_defaultCursor);
        }
    }
}

// key press event handler
void XWinKillPreviewWidget::keyPressEvent(QKeyEvent *event)
{
    // ESC pressed
    if (event->key() == Qt::Key_Escape) {
        // restore main window state
        auto *mw = gApp->mainWindow();
        mw->setWindowState((mw->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

        close();
    }
}

// initialize ui components
void XWinKillPreviewWidget::initUI()
{
    Qt::WindowFlags flags {};
    // frameless style
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::Window;
    // always on top
    flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    // transparent background
    setAttribute(Qt::WA_TranslucentBackground);
    // self free after close
    setAttribute(Qt::WA_DeleteOnClose, true);

    // rquest exclusive owning keyboard & mouse
    grabKeyboard();
    setMouseTracking(true);
    grabMouse();

    // styled kill cursor
    m_killCursor = QCursor(iconPathFromQrc("kill_cursor.svg"));
    // default forbid style cursor
    m_defaultCursor = QCursor(Qt::ForbiddenCursor);

    QTimer::singleShot(500, this, [=] {
        // show background window in all screens
        for (auto screen : QApplication::screens()) {
            // screen geometry
            auto geom = screen->geometry();
            // snapshot current scree
            auto pixmap = screen->grabWindow(m_wminfo->getRootWindow());
#ifdef USE_DEEPIN_WAYLAND
            if (WaylandCentered)
                pixmap = screen->grabWindow(m_windowStates.end()->windowId);
#endif   // USE_DEEPIN_WAYLAND
            pixmap = pixmap.copy(geom.x(), geom.y(), static_cast<int>(geom.width() * devicePixelRatioF()), static_cast<int>(geom.height() * devicePixelRatioF()));
            // create preview background widget for each screen
            auto *background = new XWinKillPreviewBackgroundWidget(pixmap, this);
            // update cursor on cursor updated signal
            connect(this, &XWinKillPreviewWidget::cursorUpdated, background, &XWinKillPreviewBackgroundWidget::setCursor);
            // resize background widget to current screen size
            background->resize(geom.size());
            // move background widget to current screen position
            background->move(geom.x(), geom.y());

            // keep a list of each background widget & screen
            m_backgroundList << background;
            m_screens << screen;

            // show background preview widget on each screen
            background->show();
        }
    });
}

// initialize connections (nothing to do here)
// wayland协议下建立连接
void XWinKillPreviewWidget::initConnections()
{
#ifdef USE_DEEPIN_WAYLAND
    if (WaylandCentered) {
        connect(m_connectionThreadObject, &ConnectionThread::connected, this,
                [this] {
                    m_eventQueue = new EventQueue(this);
                    m_eventQueue->setup(m_connectionThreadObject);

                    Registry *registry = new Registry(this);
                    setupRegistry(registry);
                },
                Qt::QueuedConnection);
        m_connectionThreadObject->moveToThread(m_connectionThread);
        m_connectionThread->start();

        m_connectionThreadObject->initConnection();
    }
#endif   // USE_DEEPIN_WAYLAND
}

//打印当前窗口信息接口
#ifdef USE_DEEPIN_WAYLAND
void XWinKillPreviewWidget::print_window_states(const QVector<ClientManagement::WindowState> &m_windowStates)
{
    if (WaylandCentered) {
        for (int i = 0; i < m_windowStates.count(); ++i) {
            qCDebug(app) << QDateTime::currentDateTime().toString(QLatin1String("hh:mm:ss.zzz "))
                         << "window[" << i << "]"
                         << "pid:" << m_windowStates.at(i).pid
                         << "title:" << m_windowStates.at(i).resourceName
                         << "windowId:" << m_windowStates.at(i).windowId
                         << "geometry:" << m_windowStates.at(i).geometry.x << m_windowStates.at(i).geometry.y
                         << m_windowStates.at(i).geometry.width << m_windowStates.at(i).geometry.height
                         << "isMinimized(" << m_windowStates.at(i).isMinimized << ")"
                         << "isFullScreen(" << m_windowStates.at(i).isFullScreen << ")"
                         << "isActive(" << m_windowStates.at(i).isActive << ")";
        }
    }
}
#endif   // USE_DEEPIN_WAYLAND

//wayland 注册
#ifdef USE_DEEPIN_WAYLAND
void XWinKillPreviewWidget::setupRegistry(Registry *registry)
{
    if (WaylandCentered) {
        connect(registry, &Registry::compositorAnnounced, this,
                [this, registry](quint32 name, quint32 version) {
                    m_compositor = registry->createCompositor(name, version, this);
                });

        connect(registry, &Registry::clientManagementAnnounced, this,
                [this, registry](quint32 name, quint32 version) {
                    m_clientManagement = registry->createClientManagement(name, version, this);
                    connect(m_clientManagement, &ClientManagement::windowStatesChanged, this,
                            [this] {
                                m_windowStates = getAllWindowStates();
                            });
                });

        connect(registry, &Registry::interfacesAnnounced, this,
                [this] {
                    Q_ASSERT(m_compositor);
                    Q_ASSERT(m_clientManagement);
                    m_windowStates = getAllWindowStates();
                });

        registry->setEventQueue(m_eventQueue);
        registry->create(m_connectionThreadObject);
        registry->setup();
    }
}

QVector<ClientManagement::WindowState> XWinKillPreviewWidget::getAllWindowStates()
{
    QVector<ClientManagement::WindowState> vWindowStates;

    // 能解析到displayjack的接口，优先使用dispalayjack接口获取窗口状态
    if (GetAllWindowStatesList) {
        // 使用displayjack库接口获取窗口状态
        WindowState *pStates = nullptr;
        int nCount = GetAllWindowStatesList(&pStates);
        if (nCount <= 0)
            return vWindowStates;

        for (int i = 0; i < nCount; i++) {
            WindowState *p = &pStates[i];
            ClientManagement::WindowState windowState;
            windowState.pid = p->pid;
            windowState.windowId = p->windowId;
            memcpy(windowState.resourceName, p->resourceName, sizeof(p->resourceName));
            windowState.geometry.x = p->geometry.x;
            windowState.geometry.y = p->geometry.y;
            windowState.geometry.width = p->geometry.width;
            windowState.geometry.height = p->geometry.height;
            windowState.isMinimized = p->isMinimized;
            windowState.isFullScreen = p->isFullScreen;
            windowState.isActive = p->isActive;

            vWindowStates.push_back(windowState);
        }
        free(pStates);
    } else {
        // 使用kde接口获取窗口状态
        Q_ASSERT(m_clientManagement);
        return m_clientManagement->getWindowStates();
    }

    return vWindowStates;
}

#endif   // USE_DEEPIN_WAYLAND
