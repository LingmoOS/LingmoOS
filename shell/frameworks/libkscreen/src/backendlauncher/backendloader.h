/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QDBusContext>
#include <QObject>

namespace KScreen
{
class AbstractBackend;
}

class QPluginLoader;
class BackendDBusWrapper;

class BackendLoader : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen")

public:
    explicit BackendLoader();
    ~BackendLoader() override;

    bool init();

    Q_INVOKABLE QString backend() const;
    Q_INVOKABLE bool requestBackend(const QString &name, const QVariantMap &arguments);
    Q_INVOKABLE void quit();

private:
    KScreen::AbstractBackend *loadBackend(const QString &name, const QVariantMap &arguments);

private:
    QPluginLoader *mLoader = nullptr;
    BackendDBusWrapper *mBackend = nullptr;
};
