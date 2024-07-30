/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_FOLDERPARSER_H
#define ATTICA_FOLDERPARSER_H

#include "folder.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Folder::Parser : public Attica::Parser<Folder>
{
private:
    Folder parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
