/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

// Self
#include "Module.h"

// Qt
#include <QHash>

// Utils
#include <utils/d_ptr_implementation.h>

// Local
#include "DebugApplication.h"

class Module::Private
{
public:
    static QHash<QString, QObject *> s_modules;
};

QHash<QString, QObject *> Module::Private::s_modules;

Module::Module(const QString &name, QObject *parent)
    : QObject(parent)
    , d()
{
    if (!name.isEmpty()) {
        Private::s_modules[name] = this;
    }
}

Module::~Module()
{
}

QObject *Module::get(const QString &name)
{
    Q_ASSERT(!name.isEmpty());

    if (Private::s_modules.contains(name)) {
        qCDebug(KAMD_LOG_APPLICATION) << "Returning a valid module object for:" << name;
        return Private::s_modules[name];
    }

    qCWarning(KAMD_LOG_APPLICATION) << "The requested module doesn't exist:" << name;
    return nullptr;
}

QHash<QString, QObject *> &Module::get()
{
    return Private::s_modules;
}

bool Module::isFeatureEnabled(const QStringList &feature) const
{
    Q_UNUSED(feature);
    return false;
}

bool Module::isFeatureOperational(const QStringList &feature) const
{
    Q_UNUSED(feature);
    return false;
}

void Module::setFeatureEnabled(const QStringList &feature, bool value)
{
    Q_UNUSED(feature);
    Q_UNUSED(value);
}

QStringList Module::listFeatures(const QStringList &feature) const
{
    Q_UNUSED(feature);
    return QStringList();
}

QDBusVariant Module::featureValue(const QStringList &property) const
{
    Q_UNUSED(property);

    return QDBusVariant();
}

void Module::setFeatureValue(const QStringList &property, const QDBusVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);
}

#include "moc_Module.cpp"
