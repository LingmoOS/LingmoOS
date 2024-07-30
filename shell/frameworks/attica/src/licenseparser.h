/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_LICENSE_PARSER_H
#define ATTICA_LICENSE_PARSER_H

#include "license.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN License::Parser : public Attica::Parser<License>
{
private:
    License parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
