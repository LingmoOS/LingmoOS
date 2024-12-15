// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-treeland-ocean-shell-v1.h"

#include <QtWaylandClient/QWaylandClientExtension>

namespace dock
{
class TreeLandMultitaskviewWorker;
class TreeLandMultitaskview : public QWaylandClientExtensionTemplate<TreeLandMultitaskview>, public QtWayland::treeland_ocean_shell_manager_v1
{
    Q_OBJECT

public:
    explicit TreeLandMultitaskview();
    void toggle();

private:
    QScopedPointer<TreeLandMultitaskviewWorker> m_worker;
};

class TreeLandMultitaskviewWorker : public QWaylandClientExtensionTemplate<TreeLandMultitaskviewWorker>, public QtWayland::treeland_multitaskview_v1
{
    Q_OBJECT

public:
    explicit TreeLandMultitaskviewWorker(struct ::treeland_multitaskview_v1 *object);
};
}
