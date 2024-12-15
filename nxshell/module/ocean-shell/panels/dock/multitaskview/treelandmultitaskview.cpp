// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmultitaskview.h"

namespace dock
{
TreeLandMultitaskview::TreeLandMultitaskview()
    : QWaylandClientExtensionTemplate<TreeLandMultitaskview>(treeland_ocean_shell_manager_v1_interface.version)
{
}

void TreeLandMultitaskview::toggle()
{
    if (!isActive()) {
        return;
    }

    if (m_worker.isNull()) {
        m_worker.reset(new TreeLandMultitaskviewWorker(get_treeland_multitaskview()));
    }
    m_worker->toggle();
}

TreeLandMultitaskviewWorker::TreeLandMultitaskviewWorker(struct ::treeland_multitaskview_v1 *object)
    : QWaylandClientExtensionTemplate<TreeLandMultitaskviewWorker>(treeland_multitaskview_v1_interface.version)
    , QtWayland::treeland_multitaskview_v1(object)
{
}
}
