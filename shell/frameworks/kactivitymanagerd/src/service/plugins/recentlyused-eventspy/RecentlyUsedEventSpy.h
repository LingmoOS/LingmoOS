/*
 *   SPDX-FileCopyrightText: 2019 Méven Car (meven.car@kdemail.net)
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Plugin.h>
#include <memory>

class KDirWatch;

class RecentlyUsedEventSpyPlugin : public Plugin
{
    Q_OBJECT

public:
    explicit RecentlyUsedEventSpyPlugin(QObject *parent);
    ~RecentlyUsedEventSpyPlugin() override;

    bool init(QHash<QString, QObject *> &modules) override;

private Q_SLOTS:
    void fileUpdated(const QString &file);
    void addDocument(const QUrl &url, const QString &application, const QString &mimetype);

private:
    QObject *m_resources;
    std::unique_ptr<KDirWatch> m_dirWatcher;
    QDateTime m_lastUpdate;
};
