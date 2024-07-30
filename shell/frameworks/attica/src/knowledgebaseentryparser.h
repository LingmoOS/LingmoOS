/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_KNOWLEDGEBASEENTRYPARSER_H
#define ATTICA_KNOWLEDGEBASEENTRYPARSER_H

#include "knowledgebaseentry.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN KnowledgeBaseEntry::Parser : public Attica::Parser<KnowledgeBaseEntry>
{
private:
    KnowledgeBaseEntry parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
