// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandlockscreen.h"

DS_BEGIN_NAMESPACE
namespace shutdown
{
TreeLandLockScreen::TreeLandLockScreen()
    : QWaylandClientExtensionTemplate<TreeLandLockScreen>(treeland_ocean_shell_manager_v1_interface.version)
{
}

void TreeLandLockScreen::lock()
{
    if (auto impl = workder())
        impl->lock();
}

void TreeLandLockScreen::shutdown()
{
    if (auto impl = workder())
        impl->shutdown();
}

void TreeLandLockScreen::switchUser()
{
    if (auto impl = workder())
        impl->switch_user();
}

TreeLandLockScreenWorker *TreeLandLockScreen::workder()
{
    if (!isActive()) {
        return nullptr;
    }

    if (m_worker.isNull()) {
        m_worker.reset(new TreeLandLockScreenWorker(get_treeland_lockscreen()));
    }
    return m_worker.get();
}

TreeLandLockScreenWorker::TreeLandLockScreenWorker(struct ::treeland_lockscreen_v1 *object)
    : QWaylandClientExtensionTemplate<TreeLandLockScreenWorker>(treeland_lockscreen_v1_interface.version)
    , QtWayland::treeland_lockscreen_v1(object)
{
}
}
DS_END_NAMESPACE
