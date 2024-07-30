/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_REMOTEACCOUNTPARSER_H
#define ATTICA_REMOTEACCOUNTPARSER_H

#include "parser.h"
#include "remoteaccount.h"

namespace Attica
{
class Q_DECL_HIDDEN RemoteAccount::Parser : public Attica::Parser<RemoteAccount>
{
private:
    RemoteAccount parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
