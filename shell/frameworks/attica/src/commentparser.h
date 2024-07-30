/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_COMMENTPARSER_H
#define ATTICA_COMMENTPARSER_H

#include "comment.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Comment::Parser : public Attica::Parser<Comment>
{
private:
    Comment parseXml(QXmlStreamReader &xml) override;
    QList<Comment> parseXmlChildren(QXmlStreamReader &xml);
    QStringList xmlElement() const override;
};

}

#endif
