/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_ACHIEVEMENTPARSER_H
#define ATTICA_ACHIEVEMENTPARSER_H

#include "achievement.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Achievement::Parser : public Attica::Parser<Achievement>
{
private:
    Achievement parseXml(QXmlStreamReader &xml) override;
    QStringList parseXmlOptions(QXmlStreamReader &xml);
    QStringList parseXmlDependencies(QXmlStreamReader &xml);
    QVariant parseXmlProgress(QXmlStreamReader &xml);
    QStringList xmlElement() const override;
};

}

#endif
