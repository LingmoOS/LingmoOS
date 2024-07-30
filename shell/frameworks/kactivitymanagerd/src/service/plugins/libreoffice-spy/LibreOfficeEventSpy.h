/*
 *   SPDX-FileCopyrightText: 2023 Méven Car (meven@kde.org)
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Plugin.h>
#include <memory>

class KDirWatch;

class LibreOfficeEventSpyPlugin : public Plugin
{
    Q_OBJECT

public:
    explicit LibreOfficeEventSpyPlugin(QObject *parent);
    ~LibreOfficeEventSpyPlugin() override;

    bool init(QHash<QString, QObject *> &modules) override;

private Q_SLOTS:
    void fileUpdated(const QString &file);
    void addDocument(const QUrl &url, const QString &application, const QString &mimetype);

private:
    QObject *m_resources;
    std::unique_ptr<KDirWatch> m_dirWatcher;
    QUrl m_lastUrl;
    bool m_init = false;
};
