// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBASEFILEWATCHER_P_H
#define DBASEFILEWATCHER_P_H

#include "base/private/dobject_p.h"

#include <QUrl>

DCORE_BEGIN_NAMESPACE

class DBaseFileWatcher;
class DBaseFileWatcherPrivate : public DObjectPrivate
{
public:
    DBaseFileWatcherPrivate(DBaseFileWatcher *qq);

    virtual bool start() = 0;
    virtual bool stop() = 0;

    QUrl url;
    bool started = false;
    static QList<DBaseFileWatcher *> watcherList;

    D_DECLARE_PUBLIC(DBaseFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DBASEFILEWATCHER_P_H
