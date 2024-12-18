// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDLOG_H
#define DDLOG_H

#include <DLog>
#include <DConfig>
#include <dtkcore_global.h>

DCORE_USE_NAMESPACE

namespace DDLog {
   inline Q_LOGGING_CATEGORY(appLog,"org.lingmo.devicemanager");
   inline Q_LOGGING_CATEGORY(deviceControlLog,"org.lingmo.devicecontrol");
   inline Q_LOGGING_CATEGORY(deviceInfoLog,"org.lingmo.deviceinfo");
}

#endif
