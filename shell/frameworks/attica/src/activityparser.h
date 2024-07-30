/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_ACTIVITYPARSER_H
#define ATTICA_ACTIVITYPARSER_H

#include "activity.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Activity::Parser : public Attica::Parser<Activity>
{
private:
    Activity parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
