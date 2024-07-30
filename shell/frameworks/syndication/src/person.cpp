/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "person.h"

namespace Syndication
{
Person::~Person()
{
}

QString Person::debugInfo() const
{
    QString info = QLatin1String("# Person begin ####################\n");

    QString dname = name();

    if (!dname.isNull()) {
        info += QLatin1String("name: #") + dname + QLatin1String("#\n");
    }

    QString duri = uri();

    if (!duri.isNull()) {
        info += QLatin1String("uri: #") + duri + QLatin1String("#\n");
    }

    QString demail = email();

    if (!demail.isNull()) {
        info += QLatin1String("email: #") + demail + QLatin1String("#\n");
    }

    info += QLatin1String("# Person end ######################\n");

    return info;
}

bool Person::operator==(const Person &other) const
{
    return name() == other.name() && email() == other.email() && uri() == other.uri();
}

} // namespace Syndication
