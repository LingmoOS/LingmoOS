/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PRIVATEDATAPARSER_H
#define ATTICA_PRIVATEDATAPARSER_H

#include "parser.h"
#include "privatedata.h"

namespace Attica
{
class Q_DECL_HIDDEN PrivateData::Parser : public Attica::Parser<PrivateData>
{
private:
    PrivateData parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
