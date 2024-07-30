/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_HOMEPAGETYPE_PARSER_H
#define ATTICA_HOMEPAGETYPE_PARSER_H

#include "homepagetype.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN HomePageType::Parser : public Attica::Parser<HomePageType>
{
private:
    HomePageType parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
