/*
 *   SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <Config.h>

#include <functional>

#include <QStandardPaths>

#include <KDirWatch>

#include <utils/d_ptr_implementation.h>

class Config::Private
{
public:
    Private(Config *parent)
        : q(parent)
        , mainConfigFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/kactivitymanagerdrc"))
        , pluginConfigFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/kactivitymanagerd-pluginsrc"))
    {
        using namespace std::placeholders;

        watcher.addFile(mainConfigFile);
        watcher.addFile(pluginConfigFile);

        QObject::connect(&watcher, &KDirWatch::created, q, std::bind(&Private::configFileChanged, this, _1));
        QObject::connect(&watcher, &KDirWatch::dirty, q, std::bind(&Private::configFileChanged, this, _1));
    }

    void configFileChanged(const QString &file)
    {
        if (file == pluginConfigFile) {
            Q_EMIT q->pluginConfigChanged();
        } else {
            Q_EMIT q->mainConfigChanged();
        }
    }

    KDirWatch watcher;

private:
    Config *const q;

    const QString mainConfigFile;
    const QString pluginConfigFile;
};

Config::Config(QObject *parent)
    : Module(QStringLiteral("config"), parent)
    , d(this)
{
}

Config::~Config()
{
}

#include "moc_Config.cpp"
