// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMPATIBILITYMANAGER_H
#define COMPATIBILITYMANAGER_H

#include <QFileSystemWatcher>
#include <QFile>
#include <tuple>
#include <any>
#include "iniParser.h"

class CompatibilityDesktopEntry{
public:
    enum ValueKey{
        Unknown,
        Exec,
        Env
    };

    using Entry = std::tuple<QString, QStringList>;

    [[nodiscard]] std::optional<Entry> group(const QString &key) const noexcept;
    [[nodiscard]] std::any value(const QString &key, const ValueKey &valueKey) const noexcept;
    void insert(const QString &groupKey, const Entry &entry) noexcept;


    QMap<QString, Entry> m_entrys;
};

class CompatibilityManager : public QObject
{
    Q_OBJECT
public:
    CompatibilityManager();
    std::optional<QString> getExec(const QString &desktopId, const QString &groupId);
    QStringList getEnv(const QString &desktopId, const QString &groupId);

private:
    void loadCompatibilityConfig();
     [[nodiscard]] ParserError parse(QFile &file) noexcept;

private:
    QFileSystemWatcher m_watcher;
    QMap<QString,CompatibilityDesktopEntry> m_compatibilityConfig;
};

#endif  // COMPATIBILITYMANAGER_H
