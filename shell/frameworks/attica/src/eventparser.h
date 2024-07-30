/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_EVENTPARSER_H
#define ATTICA_EVENTPARSER_H

#include "event.h"
#include "parser.h"

namespace Attica
{
class Q_DECL_HIDDEN Event::Parser : public Attica::Parser<Event>
{
private:
    Event parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

}

#endif
