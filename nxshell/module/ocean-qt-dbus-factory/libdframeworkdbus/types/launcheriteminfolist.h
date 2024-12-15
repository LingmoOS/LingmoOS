// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "launcheriteminfo.h"

#include <QtCore/QList>
#include <QDBusMetaType>

typedef QList<LauncherItemInfo> LauncherItemInfoList;

Q_DECLARE_METATYPE(LauncherItemInfoList)

void registerLauncherItemInfoListMetaType();
