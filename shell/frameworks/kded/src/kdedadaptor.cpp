/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdedadaptor.h"
#include "kded.h"
#include <KSharedConfig>
#include <QCoreApplication>
#include <QDBusMessage>

KdedAdaptor::KdedAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool KdedAdaptor::isModuleAutoloaded(const QString &module)
{
    return Kded::self()->isModuleAutoloaded(module);
}

bool KdedAdaptor::isModuleLoadedOnDemand(const QString &module)
{
    return Kded::self()->isModuleLoadedOnDemand(module);
}

bool KdedAdaptor::loadModule(const QString &module)
{
    return Kded::self()->loadModule(module, false) != nullptr;
}

bool KdedAdaptor::unloadModule(const QString &module)
{
    return Kded::self()->unloadModule(module);
}

void KdedAdaptor::registerWindowId(qlonglong windowId, const QDBusMessage &msg)
{
    Kded::self()->registerWindowId(windowId, msg.service());
}

void KdedAdaptor::setModuleAutoloading(const QString &module, bool autoload)
{
    return Kded::self()->setModuleAutoloading(module, autoload);
}
void KdedAdaptor::unregisterWindowId(qlonglong windowId, const QDBusMessage &msg)
{
    Kded::self()->unregisterWindowId(windowId, msg.service());
}

QStringList KdedAdaptor::loadedModules()
{
    return Kded::self()->loadedModules();
}

void KdedAdaptor::reconfigure()
{
    KSharedConfig::openConfig(QStringLiteral("kded5rc"))->reparseConfiguration();
    Kded::self()->initModules();
}

void KdedAdaptor::quit()
{
    QCoreApplication::instance()->quit();
}

#include "moc_kdedadaptor.cpp"
