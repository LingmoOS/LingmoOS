/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_BUILDSERVICEPARSER_H
#define ATTICA_BUILDSERVICEPARSER_H

#include "buildservice.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN BuildService::Parser : public Attica::Parser<BuildService>
{
private:
    BuildService parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
