// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGMANAGER_P_H
#define DCONFIGMANAGER_P_H

#include <dtkcore_global.h>
#include <QMap>
#include <QReadWriteLock>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class DConfigManager;
class DConfigManagerPrivate
{
    friend class DConfigManager;
    DConfigManager *q { nullptr };

    QMap<QString, DTK_NAMESPACE::DCORE_NAMESPACE::DConfig *> configs;
    QReadWriteLock lock;

public:
    explicit DConfigManagerPrivate(DConfigManager *qq)
        : q(qq) {}
};

#endif   // DCONFIGMANAGER_P_H
