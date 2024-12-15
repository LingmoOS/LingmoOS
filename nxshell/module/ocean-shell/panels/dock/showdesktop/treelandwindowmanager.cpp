// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandwindowmanager.h"
#include "qwayland-treeland-window-management-v1.h"
#include "wayland-treeland-window-management-v1-client-protocol.h"

namespace dock
{
TreelandWindowManager::TreelandWindowManager(QObject *parent)
    : QWaylandClientExtensionTemplate<TreelandWindowManager>(treeland_window_management_v1_interface.version)
    , m_desktopState(desktop_state_normal)
{
    setParent(parent);
}

void TreelandWindowManager::desktopToggle()
{
    if (isActive()) {
        set_desktop(m_desktopState == desktop_state_show ? desktop_state_normal : desktop_state_show);
    }
}

void TreelandWindowManager::treeland_window_management_v1_show_desktop(uint32_t state)
{
    if (state != m_desktopState) {
        m_desktopState = state;
    }
}
}
