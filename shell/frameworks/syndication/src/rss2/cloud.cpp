/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "cloud.h"

#include <QString>

namespace Syndication
{
namespace RSS2
{
Cloud::Cloud()
    : ElementWrapper()
{
}

Cloud::Cloud(const QDomElement &element)
    : ElementWrapper(element)
{
}

QString Cloud::domain() const
{
    return attribute(QStringLiteral("domain"));
}

int Cloud::port() const
{
    if (hasAttribute(QStringLiteral("port"))) {
        bool ok;
        int c = attribute(QStringLiteral("port")).toInt(&ok);
        return ok ? c : -1;
    }

    return -1;
}

QString Cloud::path() const
{
    return attribute(QStringLiteral("path"));
}

QString Cloud::registerProcedure() const
{
    return attribute(QStringLiteral("registerProcedure"));
}

QString Cloud::protocol() const
{
    return attribute(QStringLiteral("protocol"));
}

QString Cloud::debugInfo() const
{
    QString info = QLatin1String("### Cloud: ###################\n");
    if (!domain().isNull()) {
        info += QLatin1String("domain: #") + domain() + QLatin1String("#\n");
    }
    if (port() != -1) {
        info += QLatin1String("port: #") + QString::number(port()) + QLatin1String("#\n");
    }
    if (!path().isNull()) {
        info += QLatin1String("path: #") + path() + QLatin1String("#\n");
    }
    if (!registerProcedure().isNull()) {
        info += QLatin1String("registerProcedure: #") + registerProcedure() + QLatin1String("#\n");
    }
    if (!protocol().isNull()) {
        info += QLatin1String("protocol: #") + protocol() + QLatin1String("#\n");
    }
    info += QLatin1String("### Cloud end ################\n");
    return info;
}

} // namespace RSS2
} // namespace Syndication
