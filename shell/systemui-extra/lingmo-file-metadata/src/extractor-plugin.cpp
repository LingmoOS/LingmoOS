/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2012 Jörg Ehrichs <joerg.ehrichs@gmx.de>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "extractor-plugin.h"

#include <QMimeDatabase>
#include <QDebug>

using namespace LingmoUIFileMetadata;

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
    QDateTime dateTime;

    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy-MM-dd"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("dd-MM-yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy-MM"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("MM-yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy.MM.dd"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("dd.MM.yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("dd MMMM yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("MM.yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy.MM"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yy"));
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, Qt::ISODate);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("dddd d MMM yyyy h':'mm':'ss AP"));
        dateTime.setTimeSpec(Qt::LocalTime);
    }
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(dateString, QStringLiteral("yyyy:MM:dd hh:mm:ss"));
        dateTime.setTimeSpec(Qt::LocalTime);
    }
    if (!dateTime.isValid()) {
        dateTime = QLocale().toDateTime(dateString, QLocale::ShortFormat);
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        dateTime = QLocale().toDateTime(dateString, QLocale::LongFormat);
        dateTime.setTimeSpec(Qt::UTC);
    }
    if (!dateTime.isValid()) {
        qWarning() << "Could not determine correct datetime format from:" << dateString;
        return QDateTime();
    }

    return dateTime;
}

/*
QStringList ExtractorPlugin::contactsFromString(const QString& string)
{
    QString cleanedString = string;
    cleanedString = cleanedString.remove(QLatin1Char('{'));
    cleanedString = cleanedString.remove(QLatin1Char('}'));

    QStringList contactStrings = string.split(QLatin1Char(','), QString::SkipEmptyParts);
    if (contactStrings.size() == 1) {
        contactStrings = string.split(QLatin1Char(';'), QString::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" ft "), QString::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" feat. "), QString::SkipEmptyParts);
    }

    if (contactStrings.size() == 1) {
        contactStrings = string.split(QStringLiteral(" feat "), QString::SkipEmptyParts);
    }

    QStringList list;
    list.reserve(contactStrings.count());
//    for (const QString& contactName : std::as_const(contactStrings)) {
    for (const QString& contactName : contactStrings) {
        list << contactName.trimmed();
    }

    return list;
}
*/

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
