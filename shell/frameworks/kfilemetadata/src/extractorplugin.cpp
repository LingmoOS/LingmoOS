/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2012 Jörg Ehrichs <joerg.ehrichs@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "extractorplugin.h"

#include "datetimeparser_p.h"

#include <QMimeDatabase>

using namespace KFileMetaData;

ExtractorPlugin::ExtractorPlugin(QObject* parent): QObject(parent)
{
}

ExtractorPlugin::~ExtractorPlugin()
{
}

//
// Helper functions
//

QDateTime ExtractorPlugin::dateTimeFromString(const QString& dateString)
{
    return KFileMetaData::Parser::dateTimeFromString(dateString);
}

QStringList ExtractorPlugin::contactsFromString(const QString& string)
{
    QString cleanedString = string;
    cleanedString = cleanedString.remove(QLatin1Char('{'));
    cleanedString = cleanedString.remove(QLatin1Char('}'));

    QStringList contactStrings = string.split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (contactStrings.size() == 1) {
        contactStrings = string.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" ft "), Qt::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" feat. "), Qt::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" feat "), Qt::SkipEmptyParts);
    }

    QStringList list;
    list.reserve(contactStrings.count());
    for (const QString& contactName : std::as_const(contactStrings)) {
        list << contactName.trimmed();
    }

    return list;
}

QString ExtractorPlugin::getSupportedMimeType(const QString& mimetype) const
{
    const QStringList allTypes = mimetypes();
    if (allTypes.contains(mimetype)) {
        return mimetype;
    }

    QMimeDatabase db;
    auto type = db.mimeTypeForName(mimetype);
    const QStringList ancestors = type.allAncestors();
    for (auto ancestor : ancestors) {
        if (allTypes.contains(ancestor)) {
            return ancestor;
        }
    }

    return QString();
}

#include "moc_extractorplugin.cpp"
