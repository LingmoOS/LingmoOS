// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-treeland-window-management-v1.h"
#include <QtWaylandClient/QWaylandClientExtension>
#include <cstdint>

namespace dock
{

class TreelandWindowManager : public QWaylandClientExtensionTemplate<TreelandWindowManager>, public QtWayland::treeland_window_management_v1
{
public:
    explicit TreelandWindowManager(QObject *parent);

    void desktopToggle();

protected:
    void treeland_window_management_v1_show_desktop(uint32_t state) override;

private:
    uint32_t m_desktopState;
};
}
