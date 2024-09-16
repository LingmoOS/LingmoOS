// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlcenterrect.h"

#include <QDBusMetaType>

void registerControlCenterRectMetaType()
{
    qRegisterMetaType<ControlCenterRect>("ControlCenterRect");
    qDBusRegisterMetaType<ControlCenterRect>();
}
