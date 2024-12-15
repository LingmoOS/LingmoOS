// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "qwayland-treeland-ocean-shell-v1.h"

#include <QtWaylandClient/QWaylandClientExtension>

DS_BEGIN_NAMESPACE
namespace shutdown
{
class TreeLandLockScreenWorker;
class TreeLandLockScreen : public QWaylandClientExtensionTemplate<TreeLandLockScreen>, public QtWayland::treeland_ocean_shell_manager_v1
{
    Q_OBJECT

public:
    explicit TreeLandLockScreen();
    void lock();
    void shutdown();
    void switchUser();

private:
    TreeLandLockScreenWorker *workder();

private:
    QScopedPointer<TreeLandLockScreenWorker> m_worker;
};

class TreeLandLockScreenWorker : public QWaylandClientExtensionTemplate<TreeLandLockScreenWorker>, public QtWayland::treeland_lockscreen_v1
{
    Q_OBJECT

public:
    explicit TreeLandLockScreenWorker(struct ::treeland_lockscreen_v1 *object);
};
}
DS_END_NAMESPACE
