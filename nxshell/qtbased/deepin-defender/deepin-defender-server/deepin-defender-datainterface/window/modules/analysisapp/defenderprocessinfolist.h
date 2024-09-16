// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defenderprocessinfo.h"

#include <QtCore/QList>
#include <QDBusMetaType>

//save all process : pid , title , desktop
typedef QList<DefenderProcessInfo> DefenderProcessInfoList;
Q_DECLARE_METATYPE(DefenderProcessInfoList);

void registerDefenderProcessInfoListMetaType();
