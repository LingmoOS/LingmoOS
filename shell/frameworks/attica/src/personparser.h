/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PERSONPARSER_H
#define ATTICA_PERSONPARSER_H

#include "parser.h"
#include "person.h"

#include "attica_export.h"

namespace Attica
{
// exported for autotest
class ATTICA_EXPORT Person::Parser : public Attica::Parser<Person>
{
private:
    Person parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
