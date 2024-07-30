/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PUBLISHERFIELDPARSER_H
#define PUBLISHERFIELDPARSER_H

#include "parser.h"
#include "publisherfield.h"

namespace Attica
{
class Q_DECL_HIDDEN PublisherField::Parser : public Attica::Parser<PublisherField>
{
private:
    PublisherField parseXml(QXmlStreamReader &xml) override;
    QStringList xmlElement() const override;
};

} // namespace Attica

#endif // PUBLISHERFIELDPARSER_H
