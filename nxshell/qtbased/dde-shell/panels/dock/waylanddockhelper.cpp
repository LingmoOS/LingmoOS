// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylanddockhelper.h"

namespace dock {
WaylandDockHelper::WaylandDockHelper(DockPanel* panel)
    : DockHelper(panel)
{

}

void WaylandDockHelper::updateDockTriggerArea()
{
    
}

HideState WaylandDockHelper::hideState()
{
    return Show;
}
}
