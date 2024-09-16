// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockhelper.h"
#include "dsglobal.h"


namespace dock {
class WaylandDockHelper : public DockHelper
{
    Q_OBJECT

public:
    WaylandDockHelper(DockPanel* panel);
    HideState hideState() override;

public Q_SLOTS:
    void updateDockTriggerArea() override;
};
}

