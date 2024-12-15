// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop_entry_cache.h"

#include "desktop_entry_cache_updater.h"

#include <QFileInfo>
#include <QDir>
#include <DDesktopEntry>
#include <QDebug>

DCORE_USE_NAMESPACE

namespace core {
namespace process {

#define DESKTOP_ENTRY_PATH "/usr/share/applications"
#define DESKTOP_ENTRY_SUFFIX ".desktop"

DesktopEntryCache::DesktopEntryCache()
{

}

const DesktopEntry DesktopEntryCache::entryWithDesktopFile(const QString &desktopFile)
{
    QFileInfo fileInfo(desktopFile);
    if (!fileInfo.exists())
        return {};

    auto entry = DesktopEntryCacheUpdater::createEntry(fileInfo);
    if (entry) {
        m_cache[fileInfo.fileName()] = entry;
        m_cache[entry->name] = entry;
    }
    return entry;
}

QHash<QString, DesktopEntry> DesktopEntryCache::getCache()
{
    return m_cache;
}

void DesktopEntryCache::updateCache()
{
    m_cache.clear();

    QString xdgDataDirPath(getenv("XDG_DATA_DIRS"));
    if (xdgDataDirPath.isEmpty()) {
        auto fileInfoList = QDir(DESKTOP_ENTRY_PATH).entryInfoList(QDir::Files);
        for (auto &fileInfo : fileInfoList) {
            entryWithDesktopFile(fileInfo.filePath());
        }
    } else {
        QStringList xdgDataDirPaths = xdgDataDirPath.split(":", QString::SkipEmptyParts);
        for (auto path : xdgDataDirPaths) {
            auto fileInfoList = QDir(path.trimmed() + "/applications").entryInfoList(QDir::Files);
            for (auto &fileInfo : fileInfoList) {
                entryWithDesktopFile(fileInfo.filePath());
            }
        }
    }
}

} // namespace process
} // namespace core
