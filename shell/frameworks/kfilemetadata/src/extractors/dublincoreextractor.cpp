/*
    Helper class to extract XML encoded Dublin Core metadata

    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "dublincoreextractor.h"
#include "extractionresult.h"

#include "datetimeparser_p.h"

namespace {

inline QString dcNS()      { return QStringLiteral("http://purl.org/dc/elements/1.1/"); }
inline QString dctermsNS() { return QStringLiteral("http://purl.org/dc/terms/"); }

}

namespace KFileMetaData
{

void DublinCoreExtractor::extract(ExtractionResult* result, const QDomNode& fragment)
{

    for (auto e = fragment.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        // Dublin Core
        // According to http://dublincore.org/documents/dces/, the
        // properties should be treated the same regardless if
        // used in the legacy DCES or DCMI-TERMS variant

        const QString namespaceURI = e.namespaceURI();
        if (namespaceURI != dcNS() && namespaceURI != dctermsNS()) {
            continue;
        }

        if (e.text().isEmpty()) {
            continue;
        }

        const QString localName = e.localName();
        if (localName == QLatin1String("description")) {
            result->add(Property::Description, e.text());
        } else if (localName == QLatin1String("subject")) {
            result->add(Property::Subject, e.text());
        } else if (localName == QLatin1String("title")) {
            result->add(Property::Title, e.text());
        } else if (localName == QLatin1String("creator")) {
            result->add(Property::Author, e.text());
        } else if (localName == QLatin1String("created")) {
            QDateTime dt = Parser::dateTimeFromString(e.text());
            result->add(Property::CreationDate, dt);
        } else if (localName == QLatin1String("language")) {
            result->add(Property::Language, e.text());
        }
    }
}

} // namespace KFileMetaData
