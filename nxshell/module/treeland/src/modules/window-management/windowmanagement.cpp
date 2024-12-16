// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "windowmanagement.h"

#include "impl/window_management_impl.h"

#include <wserver.h>

#include <qwdisplay.h>

#include <QDebug>
#include <QQmlInfo>

extern "C" {
#include <wayland-server-core.h>
}

WindowManagementV1::WindowManagementV1(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DesktopState>("DesktopState");
}

WindowManagementV1::DesktopState WindowManagementV1::desktopState()
{
    // TODO: When the protocol is not initialized,
    // qml calls the current interface m_handle is empty
    return m_handle ? static_cast<DesktopState>(m_handle->state) : DesktopState::Normal;
}

void WindowManagementV1::setDesktopState(DesktopState state)
{
    uint32_t s = 0;
    switch (state) {
    case DesktopState::Normal:
        s = WINDOW_MANAGEMENT_V1_DESKTOP_STATE_NORMAL;
        break;
    case DesktopState::Show:
        s = WINDOW_MANAGEMENT_V1_DESKTOP_STATE_SHOW;
        break;
    case DesktopState::Preview:
        s = WINDOW_MANAGEMENT_V1_DESKTOP_STATE_PREVIEW_SHOW;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    m_handle->set_desktop(s);
    Q_EMIT desktopStateChanged();

    qmlWarning(this) << QString("Try to show desktop state (%1)!").arg(s);
}

void WindowManagementV1::create(WServer *server)
{
    m_handle = treeland_window_management_v1::create(server->handle());

    connect(m_handle, &treeland_window_management_v1::requestShowDesktop, [this](uint32_t state) {
        DesktopState s = static_cast<DesktopState>(state);
        setDesktopState(s);
    });
}

void WindowManagementV1::destroy(WServer *server) { }

wl_global *WindowManagementV1::global() const
{
    return m_handle->global;
}
