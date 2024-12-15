// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DWMSUPPORT_H
#define DWMSUPPORT_H

#include <QtGlobal>

#include "global.h"

DPP_USE_NAMESPACE

#ifdef Q_OS_LINUX
#include "dxcbwmsupport.h"
typedef DXcbWMSupport DWMSupport;
#elif defined(Q_OS_WIN)
#endif

#endif // DWMSUPPORT_H
