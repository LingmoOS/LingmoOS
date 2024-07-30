/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PUBLISHERPARSER_H
#define ATTICA_PUBLISHERPARSER_H

#include "parser.h"
#include "publisher.h"

namespace Attica
{
class Q_DECL_HIDDEN Publisher::Parser : public Attica::Parser<Publisher>
{
private:
    Publisher parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
