/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_CONFIGPARSER_H
#define ATTICA_CONFIGPARSER_H

#include "config.h"
#include "parser.h"

#include "attica_export.h"

namespace Attica
{
// exported for autotest
class ATTICA_EXPORT Config::Parser : public Attica::Parser<Config>
{
private:
    Config parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
