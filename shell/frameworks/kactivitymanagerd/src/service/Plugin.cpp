/*
 *   SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

// Self
#include "Plugin.h"

// KDE
#include <ksharedconfig.h>

// Utils
#include <utils/d_ptr_implementation.h>

// Local
#include "DebugApplication.h"

class Plugin::Private
{
public:
    Private()
        : config(nullptr)
    {
    }

    QString name;
    KSharedConfig::Ptr config;
};

Plugin::Plugin(QObject *parent)
    : Module(QString(), parent)
    , d()
{
}

Plugin::~Plugin()
{
}

KConfigGroup Plugin::config() const
{
    if (d->name.isEmpty()) {
        qCWarning(KAMD_LOG_APPLICATION) << "The plugin needs a name in order to have a config section";
        return KConfigGroup();
    }

    if (!d->config) {
        d->config = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerd-pluginsrc"));
    }

    return d->config->group(QStringLiteral("Plugin-") + d->name);
}

void Plugin::setName(const QString &name)
{
    Q_ASSERT_X(d->name.isEmpty(), "Plugin::setName", "The name can not be set twice");
    Q_ASSERT_X(!name.isEmpty(), "Plugin::setName", "The name can not be empty");

    qCDebug(KAMD_LOG_APPLICATION) << "Setting the name of " << (void *)this << " to " << name;
    d->name = name;
}

QString Plugin::name() const
{
    return d->name;
}

bool Plugin::init(QHash<QString, QObject *> &modules)
{
    if (!name().isEmpty()) {
        modules[name()] = this;
    }

    return true;
}

#include "moc_Plugin.cpp"
