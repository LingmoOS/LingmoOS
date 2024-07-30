/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_BUILDSERVICEJOBOUTPUTPARSER_H
#define ATTICA_BUILDSERVICEJOBOUTPUTPARSER_H

#include "buildservicejoboutput.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN BuildServiceJobOutput::Parser : public Attica::Parser<BuildServiceJobOutput>
{
public:
    BuildServiceJobOutput parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

} // namespace Attica

#endif // ATTICA_BUILDSERVICEJOBOUTPUTPARSER_H
