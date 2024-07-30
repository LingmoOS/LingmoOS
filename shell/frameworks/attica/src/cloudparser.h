/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2012 Laszlo Papp <lpapp@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_CLOUDPARSER_H
#define ATTICA_CLOUDPARSER_H

#include "cloud.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Cloud::Parser : public Attica::Parser<Cloud>
{
private:
    Cloud parseXml(QXmlStreamReader &xml);
    QStringList xmlElement() const;
};

}

#endif
