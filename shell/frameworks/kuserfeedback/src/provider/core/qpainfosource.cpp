/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "qpainfosource.h"

#include <QGuiApplication>
#include <QVariant>

using namespace KUserFeedback;

QPAInfoSource::QPAInfoSource() :
    AbstractDataSource(QStringLiteral("qpa"), Provider::BasicSystemInformation)
{
}

QString QPAInfoSource::description() const
{
    return tr("The Qt platform abstraction plugin.");
}

QVariant QPAInfoSource::data()
{
    QVariantMap m;
    m.insert(QStringLiteral("name"), QGuiApplication::platformName());
    return m;
}

QString QPAInfoSource::name() const
{
    return tr("QPA information");
}
