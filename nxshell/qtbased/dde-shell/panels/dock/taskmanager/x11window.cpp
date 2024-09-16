// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "x11window.h"
#include "abstractwindow.h"
#include "dsglobal.h"
#include "x11utils.h"

#include <mutex>

#include <QRect>
#include <QObject>
#include <QLoggingCategory>

#define X11 X11Utils::instance()

Q_LOGGING_CATEGORY(x11windowLog, "dde.shell.dock.taskmanager.x11window")

namespace dock {
X11Window::X11Window(xcb_window_t winid, QObject *parent)
    : AbstractWindow(parent)
    , m_windowID(winid)
    , m_pid(0)
{
    qCDebug(x11windowLog()) << "x11 window created";
}

X11Window::~X11Window()
{
    qCDebug(x11windowLog()) << "x11 window destroyed";
}

uint32_t X11Window::id()
{
    return m_windowID;
}

pid_t X11Window::pid()
{
    if (m_pid == 0) {
        updatePid();
    }

    return m_pid;
}

QString X11Window::icon()
{
    if (m_icon.isEmpty()) {
        updateIcon();
    }

    return m_icon;
}

QString X11Window::title()
{
    if (m_title.isEmpty()) {
        updateTitle();
    }

    return m_title;
}

bool X11Window::isActive()
{
    checkWindowState();
    return m_windowStates.contains(X11->getAtomByName("_NET_WM_STATE_FOCUSED"));
}

bool X11Window::shouldSkip()
{
    checkWindowTypes(), checkWindowState();
    if (hasWmStateSkipTaskBar())
        return true;

    for (auto atom : m_windowTypes) {
        if (atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_DIALOG") && !isActionMinimizeAllowed())
            return true;

        if (atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_UTILITY")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_COMBO")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_DESKTOP")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_DND")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_DOCK")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_MENU")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_NOTIFICATION")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_POPUP_MENU")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_SPLASH")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_TOOLBAR")
        || atom == X11->getAtomByName("_NET_WM_WINDOW_TYPE_TOOLTIP"))
            return true; 
    }

    return false;
}

bool X11Window::isMinimized()
{
    checkWindowState();
    return m_windowStates.contains(X11->getAtomByName("_NET_WM_STATE_HIDDEN"));
}

bool X11Window::allowClose()
{
    checkWindowAllowedActions();
    if ((m_motifWmHints.flags & MotifHintFunctions) == 0
        || (m_motifWmHints.functions & MotifFunctionAll) != 0
        || (m_motifWmHints.functions & MotifFunctionClose) != 0)
        return true;
    return m_windowAllowedActions.contains(X11->getAtomByName("_NET_WM_ACTION_CLOSE"));
}

bool X11Window::isAttention()
{
    return m_windowStates.contains(X11->getAtomByName("_NET_WM_STATE_DEMANDS_ATTENTION"));
}

void X11Window::close()
{
    X11->closeWindow(m_windowID);
}

void X11Window::activate()
{
    X11->setActiveWindow(m_windowID);
}

void X11Window::maxmize()
{
    X11->maxmizeWindow(m_windowID);
}

void X11Window::minimize()
{
    X11->minimizeWindow(m_windowID);
}

void X11Window::killClient()
{
    X11->killClient(m_windowID);
}

void X11Window::setWindowIconGeometry(const QWindow* baseWindow, const QRect& gemeotry)
{
    auto pos = baseWindow->position() + gemeotry.topLeft();
    X11->setWindowIconGemeotry(m_windowID, QRect(pos.x(), pos.y(), gemeotry.width(), gemeotry.height()));
}

void X11Window::updatePid()
{
    auto oldPid = m_pid;
    m_pid = X11->getWindowPid(m_windowID);
    if (oldPid != m_pid)
        Q_EMIT AbstractWindow::pidChanged();
}

void X11Window::updateIcon()
{
    auto oldIcon = m_icon;
    m_icon = X11->getWindowIcon(m_windowID);
    if (oldIcon != m_icon)
        Q_EMIT AbstractWindow::iconChanged();
}

void X11Window::updateTitle()
{
    auto oldTitle = m_title;
    m_title = X11->getWindowName(m_windowID);
    if (oldTitle != m_title)
        Q_EMIT AbstractWindow::titleChanged();
}

void X11Window::updateIsActive()
{
    updateWindowState();
    Q_EMIT isActiveChanged();
}

void X11Window::updateShouldSkip()
{
    updateWindowTypes();
}

void X11Window::updateAllowClose()
{
    updateWindowAllowedActions();
}

void X11Window::updateIsMinimized()
{
    updateWindowState();
}

void X11Window::updateMotifWmHints()
{
    m_motifWmHints = X11->getWindowMotifWMHints(m_windowID);
}

void X11Window::updateWindowState()
{
    m_windowStates.clear();
    m_windowStates = X11->getWindowState(m_windowID);
    Q_EMIT stateChanged();
}

void X11Window::checkWindowState()
{
    std::call_once(m_windowStateFlag, [this](){
        updateWindowState();
    });
}

void X11Window::updateWindowAllowedActions()
{
    m_windowAllowedActions.clear();
    m_windowAllowedActions = X11->getWindowAllowedActions(m_windowID);
}

void X11Window::checkWindowAllowedActions()
{
    std::call_once(m_windowAllowedActionsFlag, [this](){
        updateMotifWmHints();
        updateWindowAllowedActions();
    });
}

void X11Window::updateWindowTypes()
{
    m_windowTypes.clear();
    m_windowTypes = X11->getWindowTypes(m_windowID);
}

void X11Window::checkWindowTypes()
{
    std::call_once(m_windowTypeFlag, [this](){
        updateWindowTypes();
    });
}

bool X11Window::hasWmStateModal()
{
    checkWindowState();
    return m_windowStates.contains(X11->getAtomByName("_NET_WM_STATE_MODAL"));
}

bool X11Window::hasWmStateSkipTaskBar()
{
    checkWindowState();
    return m_windowStates.contains(X11->getAtomByName("_NET_WM_STATE_SKIP_TASKBAR"));
}

bool X11Window::isActionMinimizeAllowed()
{
    checkWindowAllowedActions();
    return m_windowAllowedActions.contains(X11->getAtomByName("_NET_WM_ACTION_MINIMIZE"));
}

}
