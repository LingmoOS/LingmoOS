/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PARSER_H
#define ATTICA_PARSER_H

#include <QStringList>
#include <QXmlStreamReader>

#include "listjob.h"

namespace Attica
{
template<class T>
class ATTICA_EXPORT Parser
{
public:
    T parse(const QString &xml);
    typename T::List parseList(const QString &xml);
    Metadata metadata() const;
    virtual ~Parser();

protected:
    virtual QStringList xmlElement() const = 0;
    virtual T parseXml(QXmlStreamReader &xml) = 0;

private:
    void parseMetadataXml(QXmlStreamReader &xml);
    Metadata m_metadata;
};

}

#endif
