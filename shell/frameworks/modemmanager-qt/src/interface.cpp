/*
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "interface_p.h"

ModemManager::InterfacePrivate::InterfacePrivate(const QString &path, Interface *q)
    : uni(path)
    , q_ptr(q)
{
}

ModemManager::InterfacePrivate::~InterfacePrivate()
{
}

ModemManager::Interface::Interface(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new InterfacePrivate(path, this))
{
}

ModemManager::Interface::Interface(InterfacePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

ModemManager::Interface::~Interface()
{
    delete d_ptr;
}

QString ModemManager::Interface::uni() const
{
    Q_D(const Interface);
    return d->uni;
}

void ModemManager::InterfacePrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_UNUSED(interface)
    Q_UNUSED(properties)
    Q_UNUSED(invalidatedProps)
}

#include "moc_interface.cpp"
#include "moc_interface_p.cpp"
