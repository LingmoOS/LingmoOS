/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_FORUMPARSER_H
#define ATTICA_FORUMPARSER_H

#include "forum.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Forum::Parser : public Attica::Parser<Forum>
{
private:
    Forum parseXml(QXmlStreamReader &xml) override;
    QList<Forum> parseXmlChildren(QXmlStreamReader &xml);
    QStringList xmlElement() const override;
};

}

#endif
