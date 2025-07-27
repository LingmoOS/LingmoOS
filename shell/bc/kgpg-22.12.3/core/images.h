/*
    SPDX-FileCopyrightText: 2006 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGES_H
#define IMAGES_H

#include <QIcon>

namespace KgpgCore
{

namespace Images
{
    QIcon single();
    QIcon pair();
    QIcon group();
    QIcon orphan();
    QIcon signature();
    QIcon userId();
    QIcon photo();
    QIcon revoke();

    /* Desktop image */
    QIcon kgpg();
}

} // namespace KgpgCore

#endif // IMAGES_H
