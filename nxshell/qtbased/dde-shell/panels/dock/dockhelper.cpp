// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockhelper.h"
#include "dockpanel.h"
#include "dsglobal.h"

#include <QGuiApplication>

namespace dock {

DockHelper::DockHelper(DockPanel* parent)
    : QObject(parent)
    , m_panel(parent)
{
}

DockPanel* DockHelper::parent()
{
    return m_panel;
}
}
