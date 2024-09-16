// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandwindow.h"
#include "abstractwindow.h"

#include <cstdint>
#include <sys/types.h>

#include <QLoggingCategory>

#include <private/qwaylandwindow_p.h>
#include <private/qwaylandnativeinterface_p.h>

Q_LOGGING_CATEGORY(waylandwindowLog, "dde.shell.dock.taskmanager.treelandwindow")

namespace dock {
ForeignToplevelHandle::ForeignToplevelHandle(struct ::ztreeland_foreign_toplevel_handle_v1 *object)
    : QWaylandClientExtensionTemplate<ForeignToplevelHandle>(1)
    , QtWayland::ztreeland_foreign_toplevel_handle_v1(object)
    , m_pid(0)
    , m_isReady(false)
{
    init(object);
}

uint32_t ForeignToplevelHandle::id() const
{
    return m_identifier;
}

pid_t ForeignToplevelHandle::pid() const
{
    return m_pid;
}

QString ForeignToplevelHandle::title() const
{
    return m_title;
}

QList<uint32_t> ForeignToplevelHandle::state() const
{
    return m_states;
}

bool ForeignToplevelHandle::isReady() const
{
    return m_isReady;
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_pid(uint32_t pid)
{
    if (pid == m_pid) return;

    m_pid = pid;
    Q_EMIT pidChanged();
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_title(const QString &title)
{
    if (title == m_title) return;

    m_title = title;
    Q_EMIT titleChanged();
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_app_id(const QString &app_id)
{
    if (app_id == m_appId) return;
    m_appId = app_id;
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_identifier(uint32_t identifier)
{
    if (identifier == m_identifier) return;
    m_identifier = identifier;
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_state(wl_array *state)
{
    m_states.clear();
    const uint32_t* items = reinterpret_cast<const uint32_t*>(state->data);
    int itemCount = state->size / sizeof(uint32_t);

    for (int i = 0; i < itemCount; i++) {
        m_states.append(items[i]);
    }

    Q_EMIT isActiveChanged();
}

void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_done()
{
    m_isReady = true;
    Q_EMIT handlerIsReady();
}
void ForeignToplevelHandle::ztreeland_foreign_toplevel_handle_v1_closed()
{
    Q_EMIT handlerIsDeleted();
}

TreeLandWindow::TreeLandWindow(uint32_t id, QObject *parent)
    : AbstractWindow(parent)
    , m_id(id)
{
    qCDebug(waylandwindowLog()) << "wayland window created";
}

TreeLandWindow::~TreeLandWindow()
{
    qCDebug(waylandwindowLog()) << "wayland window destoryed";
}

uint32_t TreeLandWindow::id()
{
    return m_id;
}

pid_t TreeLandWindow::pid()
{
    return m_foreignToplevelHandle ? m_foreignToplevelHandle->pid() : 0;
}

QString TreeLandWindow::icon()
{
    return "";
}

QString TreeLandWindow::title()
{
    return m_foreignToplevelHandle ? m_foreignToplevelHandle->title() : "";
}

void TreeLandWindow::updateIsActive()
{
}

bool TreeLandWindow::isActive()
{
    return m_foreignToplevelHandle->state().contains(Active);
}

bool TreeLandWindow::shouldSkip()
{
    return false;
}

bool TreeLandWindow::isMinimized()
{
    return m_foreignToplevelHandle->state().contains(Minimized);
}

bool TreeLandWindow::allowClose()
{
    return true;
}

bool TreeLandWindow::isAttention()
{
    return false;
}

void TreeLandWindow::close()
{
    m_foreignToplevelHandle->close();
}

void TreeLandWindow::activate()
{
    QtWaylandClient::QWaylandNativeInterface *app =
            static_cast<QtWaylandClient::QWaylandNativeInterface *>(
                    QGuiApplication::platformNativeInterface());
    auto seat = app->seat();
    m_foreignToplevelHandle->activate(seat);
}

void TreeLandWindow::maxmize()
{
    m_foreignToplevelHandle->set_maximized();
}

void TreeLandWindow::minimize()
{
    m_foreignToplevelHandle->set_minimized();
}

void TreeLandWindow::killClient()
{
    m_foreignToplevelHandle->close();
}

void TreeLandWindow::setWindowIconGeometry(const QWindow* baseWindow, const QRect& gemeotry)
{
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(baseWindow->handle());
    m_foreignToplevelHandle->set_rectangle(waylandWindow->surface(), gemeotry.x(), gemeotry.y(), gemeotry.width(), gemeotry.height());
}

void TreeLandWindow::updatePid()
{

}
void TreeLandWindow::updateIcon()
{

}

void TreeLandWindow::updateTitle()
{

}

void TreeLandWindow::updateShouldSkip()
{

}

void TreeLandWindow::updateAllowClose()
{

}

void TreeLandWindow::updateIsMinimized()
{

}

bool TreeLandWindow::isReady()
{
    return m_foreignToplevelHandle->isReady();
}

void TreeLandWindow::setForeignToplevelHandle(ForeignToplevelHandle* handle)
{
    if (m_foreignToplevelHandle && m_foreignToplevelHandle.get() == handle) return;
    m_foreignToplevelHandle.reset(handle);
    m_id = m_foreignToplevelHandle->id();

    connect(m_foreignToplevelHandle.get(), &ForeignToplevelHandle::pidChanged, this, &AbstractWindow::pidChanged);
    connect(m_foreignToplevelHandle.get(), &ForeignToplevelHandle::titleChanged, this, &AbstractWindow::titleChanged);
    connect(m_foreignToplevelHandle.get(), &ForeignToplevelHandle::isActiveChanged, this, &AbstractWindow::isActiveChanged);
}

}
