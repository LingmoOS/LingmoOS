/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "constants.h"

#include <QString>

namespace Syndication
{
namespace Atom
{
QString atom1Namespace()
{
    return QStringLiteral("http://www.w3.org/2005/Atom");
}

QString atom0_3Namespace()
{
    return QStringLiteral("http://purl.org/atom/ns#");
}

} // namespace Atom
} // namespace Syndication
