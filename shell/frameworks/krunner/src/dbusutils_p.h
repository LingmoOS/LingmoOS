/*
    SPDX-FileCopyrightText: 2017, 2018 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "action.h"
#include <KRunner/QueryMatch>
#include <QDBusArgument>
#include <QList>
#include <QString>
#include <QVariantMap>

struct RemoteMatch {
    // sssida{sv}
    QString id;
    QString text;
    QString iconName;
    int categoryRelevance = qToUnderlying(KRunner::QueryMatch::CategoryRelevance::Lowest);
    qreal relevance = 0;
    QVariantMap properties;
};

typedef QList<RemoteMatch> RemoteMatches;

struct RemoteImage {
    // iiibiiay (matching notification spec image-data attribute)
    int width = 0;
    int height = 0;
    int rowStride = 0;
    bool hasAlpha = false;
    int bitsPerSample = 0;
    int channels = 0;
    QByteArray data;
};

inline QDBusArgument &operator<<(QDBusArgument &argument, const RemoteMatch &match)
{
    argument.beginStructure();
    argument << match.id;
    argument << match.text;
    argument << match.iconName;
    argument << match.categoryRelevance;
    argument << match.relevance;
    argument << match.properties;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteMatch &match)
{
    argument.beginStructure();
    argument >> match.id;
    argument >> match.text;
    argument >> match.iconName;
    argument >> match.categoryRelevance;
    argument >> match.relevance;
    argument >> match.properties;
    argument.endStructure();

    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const KRunner::Action &action)
{
    argument.beginStructure();
    argument << action.id();
    argument << action.text();
    argument << action.iconSource();
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, KRunner::Action &action)
{
    QString id;
    QString text;
    QString iconName;
    argument.beginStructure();
    argument >> id;
    argument >> text;
    argument >> iconName;
    argument.endStructure();
    action = KRunner::Action(id, iconName, text);
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const RemoteImage &image)
{
    argument.beginStructure();
    argument << image.width;
    argument << image.height;
    argument << image.rowStride;
    argument << image.hasAlpha;
    argument << image.bitsPerSample;
    argument << image.channels;
    argument << image.data;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteImage &image)
{
    argument.beginStructure();
    argument >> image.width;
    argument >> image.height;
    argument >> image.rowStride;
    argument >> image.hasAlpha;
    argument >> image.bitsPerSample;
    argument >> image.channels;
    argument >> image.data;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(QList<KRunner::Action>)
Q_DECLARE_METATYPE(RemoteMatch)
Q_DECLARE_METATYPE(RemoteMatches)
Q_DECLARE_METATYPE(RemoteImage)
