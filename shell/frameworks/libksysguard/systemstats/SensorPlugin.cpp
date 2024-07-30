/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorPlugin.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorPlugin::Private
{
public:
    QList<SensorContainer *> containers;
};

SensorPlugin::SensorPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    Q_UNUSED(args)
}

SensorPlugin::~SensorPlugin() = default;

QList<SensorContainer *> SensorPlugin::containers() const
{
    return d->containers;
}

QString SensorPlugin::providerName() const
{
    return QString();
}

void SensorPlugin::update()
{
}

void SensorPlugin::addContainer(SensorContainer *container)
{
    d->containers << container;
}
