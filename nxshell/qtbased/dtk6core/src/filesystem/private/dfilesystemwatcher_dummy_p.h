// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILESYSTEMWATCHER_WIN_P_H
#define DFILESYSTEMWATCHER_WIN_P_H

#include "base/private/dobject_p.h"

DCORE_BEGIN_NAMESPACE

class DFileSystemWatcher;
class DFileSystemWatcherPrivate : public DObjectPrivate
{
    Q_DECLARE_PUBLIC(DFileSystemWatcher)

public:
    DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq);
    ~DFileSystemWatcherPrivate();

    // private slots
    void _q_readFromInotify();
};

void DFileSystemWatcherPrivate::_q_readFromInotify()
{

}

DCORE_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_WIN_P_H
