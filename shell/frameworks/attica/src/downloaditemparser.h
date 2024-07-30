/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_DOWNLOADITEMPARSER_H
#define ATTICA_DOWNLOADITEMPARSER_H

#include "downloaditem.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN DownloadItem::Parser : public Attica::Parser<DownloadItem>
{
private:
    DownloadItem parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
