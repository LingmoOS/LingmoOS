// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screensaverwindow.h"
#include "screensaverview.h"

#include <QScreen>
#include <QTimer>
#include <QX11Info>
#include <QProcess>

#include <xcb/xcb.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

ScreenSaverWindow::ScreenSaverWindow(QObject *parent)
    : QObject(parent)
{
    m_view = new ScreenSaverView();

    // kwin 下不可添加Dialog标志，否则会导致窗口只能显示出一个，然而，在 deepin-wm 上使用其它窗口类型时又会有动画
    //! todo观察Qt::WindowDoesNotAcceptFocus的最新影响 2021-10-22
    // 不要添加Qt::WindowDoesNotAcceptFocus，以防止deepin-kwin在进入到显示桌面模式时触发屏幕保护
    // 但是却没有退出显示桌面模式，这样将会导致dock显示在屏幕保护窗口的上方
    m_view->setFlags(Qt::BypassWindowManagerHint);

    connect(m_view, &ScreenSaverView::inputEvent, this, &ScreenSaverWindow::inputEvent);
}

ScreenSaverWindow::~ScreenSaverWindow()
{
    m_view->deleteLater();
}

bool ScreenSaverWindow::start(const QString &filePath)
{
    return m_view->start(filePath);
}

void ScreenSaverWindow::stop()
{
    m_view->stop();
}

WId ScreenSaverWindow::winId() const
{
    m_view->create();

    return m_view->winId();
}

Qt::WindowFlags ScreenSaverWindow::flags() const
{
    return m_view->flags();
}

void ScreenSaverWindow::setFlags(Qt::WindowFlags flags, bool bypassWindowManager)
{
    const Qt::WindowFlags old_flags = m_view->flags();

    // 如果改变了窗口类型
    if ((old_flags & Qt::BypassWindowManagerHint) != bypassWindowManager) {
        if (m_view->handle()) {
            Q_ASSERT(!m_view->m_process || m_view->m_process->state() == QProcess::NotRunning);
            // 需要重新创建native window
            m_view->destroy();
        }
    }

    if (bypassWindowManager) {
        flags |= Qt::BypassWindowManagerHint;
    } else {
        flags &= ~Qt::BypassWindowManagerHint;

        // kwin 下不可添加Dialog标志，否则会导致窗口只能显示出一个，然而，在 deepin-wm 上使用其它窗口类型时又会有动画
        //! todo观察Qt::WindowDoesNotAcceptFocus的最新影响 2021-10-22
        // 不要添加Qt::WindowDoesNotAcceptFocus，以防止deepin-kwin在进入到显示桌面模式时触发屏幕保护
        // 但是却没有退出显示桌面模式，这样将会导致dock显示在屏幕保护窗口的上方
        flags |= Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::Drawer | Qt::WindowDoesNotAcceptFocus;
    }

    m_view->setFlags(flags);
}

QWindow::Visibility ScreenSaverWindow::visibility() const
{
    return m_view->visibility();
}

QScreen *ScreenSaverWindow::screen() const
{
    return m_view->screen();
}

void ScreenSaverWindow::setGeometry(const QRect &rect)
{
    m_view->setGeometry(rect);
}

void ScreenSaverWindow::setScreen(QScreen *screen)
{
    // 必须把窗口移动到屏幕所在位置，否则窗口被创建时会被移动到0,0所在的屏幕
    m_view->setPosition(screen->availableGeometry().center());
    m_view->setScreen(screen);

    // 确保窗口一直和屏幕大小一致
    m_view->setGeometry(screen->geometry());
    connect(screen, &QScreen::geometryChanged, this, &ScreenSaverWindow::setGeometry);
}

void ScreenSaverWindow::show()
{
    m_view->setGeometry(screen()->geometry());
    m_view->showFullScreen();

    QTimer::singleShot(500, m_view, [this] {
        // 在kwin中，窗口类型为Qt::Drawer时会导致多屏情况下只会有一个窗口被显示，另一个被最小化
        // 这里判断最小化的窗口后更改其窗口类型再次显示。

        bool is_hidden = m_view->visibility() == QWindow::Minimized;

        if (!is_hidden) {
            // KWin 上开启 HiddenPreviews=6 配置后，无法直接判断出窗口的状态，因此fallback到读取窗口属性判断其是否被隐藏
            Atom atom;
            int format;
            ulong nitems;
            ulong bytes_after_return;
            uchar *prop_datas;
            XGetWindowProperty(QX11Info::display(),
                               m_view->winId(),
                               XInternAtom(QX11Info::display(), "_NET_WM_STATE", true),
                               0, 1024, false,
                               XA_ATOM, &atom,
                               &format, &nitems,
                               &bytes_after_return, &prop_datas);

            if (prop_datas && format == 32 && atom == XA_ATOM) {
                const Atom *states = reinterpret_cast<const Atom *>(prop_datas);
                const Atom *statesEnd = states + nitems;
                if (statesEnd != std::find(states, statesEnd, XInternAtom(QX11Info::display(), "_NET_WM_STATE_HIDDEN", true)))
                    is_hidden = true;
                XFree(prop_datas);
            }
        }

        if (is_hidden) {
            m_view->setFlags((m_view->flags() & ~Qt::Dialog) | Qt::Window);
            m_view->close();
            m_view->show();
        }
    });
}

void ScreenSaverWindow::hide()
{
    m_view->hide();
}

void ScreenSaverWindow::close()
{
    m_view->close();
}
