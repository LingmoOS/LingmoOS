// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"
#include "dsglobal.h"
#include "x11utils.h"
#include "x11window.h"
#include "x11preview.h"
#include "abstractwindow.h"
#include "x11windowmonitor.h"
#include "abstractwindowmonitor.h"

#include <memory>

#include <thread>
#include <cstdint>
#include <iterator>
#include <algorithm>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <DDBusSender>

#include <QPointer>
#include <QWindow>
#include <QGuiApplication>
#include <QLoggingCategory>

#define X11 X11Utils::instance()

Q_LOGGING_CATEGORY(x11Log, "dde.shell.dock.taskmanager.x11windowmonitor")

namespace dock {
static QPointer<X11WindowMonitor> monitor;
bool XcbEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    if (eventType != "xcb_generic_event_t" || monitor.isNull())
        return false;

    auto xcb_event = reinterpret_cast<xcb_generic_event_t*>(message);
    switch (xcb_event->response_type) {
        case XCB_PROPERTY_NOTIFY: {
            auto pE = reinterpret_cast<xcb_property_notify_event_t*>(xcb_event);
            Q_EMIT monitor->windowPropertyChanged(pE->window, pE->atom);
            break;
        }
    }
    return false;
};

X11WindowMonitor::X11WindowMonitor(QObject* parent)
    : AbstractWindowMonitor(parent)
{
    monitor = this;
    connect(this, &X11WindowMonitor::windowMapped, this, &X11WindowMonitor::onWindowMapped);
    connect(this, &X11WindowMonitor::windowDestoried, this, &X11WindowMonitor::onWindowDestoried);
    connect(this, &X11WindowMonitor::windowPropertyChanged, this, &X11WindowMonitor::onWindowPropertyChanged);
}

void X11WindowMonitor::start()
{
    const xcb_setup_t *setup = xcb_get_setup(X11->getXcbConnection());
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    m_rootWindow = screen->root;

    uint32_t value_list[] = {
            0                               | XCB_EVENT_MASK_PROPERTY_CHANGE        |
            XCB_EVENT_MASK_VISIBILITY_CHANGE    | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY    |
            XCB_EVENT_MASK_STRUCTURE_NOTIFY     | XCB_EVENT_MASK_FOCUS_CHANGE
    };

    xcb_change_window_attributes(X11->getXcbConnection(), m_rootWindow, XCB_CW_EVENT_MASK, value_list);
    xcb_flush(X11->getXcbConnection());

    m_xcbEventFilter.reset(new XcbEventFilter());
    qApp->installNativeEventFilter(m_xcbEventFilter.get());
    QMetaObject::invokeMethod(this, &X11WindowMonitor::handleRootWindowClientListChanged);
}

void X11WindowMonitor::stop()
{
    qApp->removeNativeEventFilter(m_xcbEventFilter.get());
    m_xcbEventFilter.reset(nullptr);
    Q_EMIT AbstractWindowMonitor::WindowMonitorShutdown();
}


void X11WindowMonitor::clear()
{
    m_windows.clear();
    m_windowPreview.reset(nullptr);
}

QPointer<AbstractWindow> X11WindowMonitor::getWindowByWindowId(ulong windowId)
{
    return m_windows.value(windowId).get();
}

void X11WindowMonitor::presentWindows(QList<uint32_t> windows)
{
    DDBusSender().interface("com.deepin.wm")
                .path("/com/deepin/wm")
                .service("com.deepin.wm")
                .method("PresentWindows")
                .arg(windows)
                .call().waitForFinished();
}

void X11WindowMonitor::showItemPreview(const QPointer<AppItem> &item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction)
{
    // custom created preview popup window and show at (relativePositionItem.x + previewXoffset, relativePositionItem.y + previewYoffset) pos
    // direction is dock current position

    if (m_windowPreview.isNull()) {
        m_windowPreview.reset(new X11WindowPreviewContainer(this));
    }

    m_windowPreview->showPreview(item,qobject_cast<QWindow*>(relativePositionItem), previewXoffset, previewYoffset, direction);
    m_windowPreview->updatePosition();
}

void X11WindowMonitor::hideItemPreview()
{
    if (m_windowPreview.isNull()) return;
    m_windowPreview->hidePreView();
}

void X11WindowMonitor::previewWindow(uint32_t winId)
{
    DDBusSender().interface("com.deepin.wm")
            .path("/com/deepin/wm")
            .service("com.deepin.wm")
            .method("PreviewWindow")
            .arg(winId)
            .call().waitForFinished();
}

void X11WindowMonitor::cancelPreviewWindow()
{
    DDBusSender().interface("com.deepin.wm")
            .path("/com/deepin/wm")
            .service("com.deepin.wm")
            .method("CancelPreviewWindow")
            .call().waitForFinished();
}

void X11WindowMonitor::onWindowMapped(xcb_window_t xcb_window)
{
    auto window = m_windows.value(xcb_window, nullptr);
    if (window) return;
    window = QSharedPointer<X11Window>{new X11Window(xcb_window, this)};
    m_windows.insert(xcb_window, window);

    if (window->pid() == qApp->applicationPid()) return;

    uint32_t value_list[] = { XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE};
    xcb_change_window_attributes(X11->getXcbConnection(), xcb_window, XCB_CW_EVENT_MASK, value_list);
    Q_EMIT AbstractWindowMonitor::windowAdded(static_cast<QPointer<AbstractWindow>>(window.get()));
}

void X11WindowMonitor::onWindowDestoried(xcb_window_t xcb_window)
{
    auto window = m_windows.value(xcb_window, nullptr);
    if (window) {
        m_windows.remove(xcb_window);
    }
}

void X11WindowMonitor::onWindowPropertyChanged(xcb_window_t window, xcb_atom_t atom)
{
    if (window == m_rootWindow) {
        handleRootWindowPropertyNotifyEvent(atom);
        return;
    }

    auto x11Window = m_windows.value(window);
    if (!x11Window) {
        return;
    }

    if (atom == X11->getAtomByName("_NET_WM_STATE")) {
        x11Window->updateWindowState();
    }else if (atom == X11->getAtomByName("_NET_WM_PID")) {
        x11Window->updatePid();
    } else if (atom == X11->getAtomByName("_NET_WM_NAME")) {
        x11Window->updateTitle();
    } else if (atom == X11->getAtomByName("_NET_WM_ICON")) {
        x11Window->updateIcon();
    } else if (atom == X11->getAtomByName("_NET_WM_ALLOWED_ACTIONS")) {
        x11Window->updateWindowAllowedActions();
    } else if (atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE")) {
        x11Window->updateWindowTypes();
    } else if (atom == X11->getAtomByName("_MOTIF_WM_HINTS")) {
        x11Window->updateMotifWmHints();
    }

    auto appitem = x11Window->getAppItem();
    if (x11Window->shouldSkip() && appitem) {
        appitem->removeWindow(x11Window.data());
    }
}

void X11WindowMonitor::handleRootWindowPropertyNotifyEvent(xcb_atom_t atom)
{
    if (atom == X11->getAtomByName("_NET_CLIENT_LIST")) {
        handleRootWindowClientListChanged();
    }
}

void X11WindowMonitor::handleRootWindowClientListChanged()
{
    auto currentOpenedWindowList = X11->getWindowClientList(m_rootWindow);

    for (auto openedWindows : currentOpenedWindowList) {
        if (!m_windows.contains(openedWindows)) {
            onWindowMapped(openedWindows);
        }
    }

    for (auto alreadyOpenedWindow : m_windows.keys()) {
        if (!currentOpenedWindowList.contains(alreadyOpenedWindow)) {
            windowDestoried(alreadyOpenedWindow);
        }
    }

}
}
