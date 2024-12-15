// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop_entry_cache_updater.h"

#include <DDesktopEntry>
#include <QDebug>
#include <QDir>

DCORE_USE_NAMESPACE

namespace core {
namespace process {

DesktopEntryCacheUpdater::DesktopEntryCacheUpdater(QObject *parent)
    : QObject(parent)
{
}

DesktopEntry DesktopEntryCacheUpdater::createEntry(const QFileInfo &fileInfo)
{
    if (!fileInfo.exists()) {
        return {};
    }

    DDesktopEntry ocean(fileInfo.filePath());
    DesktopEntry entry(new struct desktop_entry_t());

    // display name
    entry->displayName = ocean.oceanDisplayName();
    // icon name
    auto icon = ocean.stringValue("Icon");
    if (!icon.isEmpty()) {
        entry->icon = icon;
    }

    // exec & name
    auto tryExec = ocean.stringValue("TryExec");
    auto execStr = ocean.stringValue("Exec");
    QStringList execList = execStr.split("exec", QString::SkipEmptyParts, Qt::CaseInsensitive);
    QString exec = execList.size() > 1 ? execList.last().trimmed() : execStr.trimmed();
    exec = exec.remove("\"");
    if (!tryExec.isEmpty()) {
        entry->exec = tryExec.split(' ');
        entry->name = QFileInfo(entry->exec[0]).baseName();
    } else if (!exec.isEmpty()) {
        entry->exec = exec.split(' ');
        entry->name = QFileInfo(entry->exec[0]).baseName();
    } else {
        entry->name = ocean.name().toLower();
    }
    // ocean-file-manager plugins
    if (execStr.contains("ocean-file-manager") && execStr.contains("plugin")) {
        entry->name = fileInfo.completeBaseName();
    }
    if (icon.contains("ocean-file-manager")) {
        if (ocean.stringValue("GenericName").trimmed() != "File Manager") {
            return {};
        }
        entry->exec.clear();
        entry->exec.append("ocean-file-manager");
        entry->name = "ocean-file-manager";
    }

    // startup wm class & name
    auto wmclass = ocean.stringValue("StartupWMClass").toLower();
    if (!wmclass.isEmpty()) {
        entry->startup_wm_class = wmclass;
        entry->name = wmclass;
    }

    return entry;
}

} // namespace process
} // namespace core
