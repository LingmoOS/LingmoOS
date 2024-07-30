/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_ACCOUNTBALANCEPARSER_H
#define ATTICA_ACCOUNTBALANCEPARSER_H

#include "accountbalance.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN AccountBalance::Parser : public Attica::Parser<AccountBalance>
{
private:
    AccountBalance parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
