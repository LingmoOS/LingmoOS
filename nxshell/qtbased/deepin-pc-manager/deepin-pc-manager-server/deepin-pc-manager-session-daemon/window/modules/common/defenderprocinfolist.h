// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERPROCINFOLIST_H
#define DEFENDERPROCINFOLIST_H

#include "defenderprocinfo.h"

typedef QList<DefenderProcInfo> DefenderProcInfoList;
Q_DECLARE_METATYPE(DefenderProcInfoList);

void registerDefenderProcInfoListMetaType();
#endif
