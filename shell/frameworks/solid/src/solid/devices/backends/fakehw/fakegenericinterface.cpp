/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakegenericinterface.h"

using namespace Solid::Backends::Fake;

FakeGenericInterface::FakeGenericInterface(FakeDevice *device)
    : FakeDeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(QMap<QString, int>)), this, SIGNAL(propertyChanged(QMap<QString, int>)));
    connect(device, SIGNAL(conditionRaised(QString, QString)), this, SIGNAL(conditionRaised(QString, QString)));
}

FakeGenericInterface::~FakeGenericInterface()
{
}

QVariant FakeGenericInterface::property(const QString &key) const
{
    return fakeDevice()->property(key);
}

QMap<QString, QVariant> FakeGenericInterface::allProperties() const
{
    return fakeDevice()->allProperties();
}

bool FakeGenericInterface::propertyExists(const QString &key) const
{
    return fakeDevice()->propertyExists(key);
}

#include "moc_fakegenericinterface.cpp"
