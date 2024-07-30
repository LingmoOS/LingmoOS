/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_TOPICPARSER_H
#define ATTICA_TOPICPARSER_H

#include "parser.h"
#include "topic.h"

namespace Attica
{
class Q_DECL_HIDDEN Topic::Parser : public Attica::Parser<Topic>
{
private:
    Topic parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
