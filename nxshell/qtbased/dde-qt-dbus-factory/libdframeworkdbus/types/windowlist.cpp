// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowlist.h"

#include <QtDBus>

void registerWindowListMetaType()
{
    qRegisterMetaType<WindowList>();
    qDBusRegisterMetaType<WindowList>();
}
