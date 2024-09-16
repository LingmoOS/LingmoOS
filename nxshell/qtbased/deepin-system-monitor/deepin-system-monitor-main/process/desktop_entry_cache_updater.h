// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ENTRY_CACHE_UPDATER_H
#define DESKTOP_ENTRY_CACHE_UPDATER_H

#include <QObject>
#include <QFileInfo>

#include "desktop_entry_cache.h"

namespace core {
namespace process {

class DesktopEntryCacheUpdater : public QObject
{
    Q_OBJECT
    friend class DesktopEntryCache;

public:
    explicit DesktopEntryCacheUpdater(QObject *parent = nullptr);
    ~DesktopEntryCacheUpdater() override = default;

    static DesktopEntry createEntry(const QFileInfo &fileInfo);
};

} // namespace process
} // namespace core

#endif // DESKTOP_ENTRY_CACHE_UPDATER_H
