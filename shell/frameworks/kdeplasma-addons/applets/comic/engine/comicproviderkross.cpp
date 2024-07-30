/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comicproviderkross.h"

ComicProviderKross::ComicProviderKross(QObject *parent, const KPluginMetaData &data, IdentifierType type, const QVariant &identifier)
    : ComicProvider(parent, data, type, identifier)
    , m_wrapper(this)
{
}

ComicProviderKross::~ComicProviderKross()
{
}

bool ComicProviderKross::isLeftToRight() const
{
    return m_wrapper.isLeftToRight();
}

bool ComicProviderKross::isTopToBottom() const
{
    return m_wrapper.isTopToBottom();
}

IdentifierType ComicProviderKross::identifierType() const
{
    return m_wrapper.identifierType();
}

QUrl ComicProviderKross::websiteUrl() const
{
    return QUrl(m_wrapper.websiteUrl());
}

QUrl ComicProviderKross::shopUrl() const
{
    return QUrl(m_wrapper.shopUrl());
}

QImage ComicProviderKross::image() const
{
    return m_wrapper.comicImage();
}

QString ComicProviderKross::identifierToString(const QVariant &identifier) const
{
    QString result;

    if (!identifier.isNull() && identifier.typeId() != QMetaType::Bool) {
        if (identifierType() == IdentifierType::DateIdentifier) {
            result = identifier.toDate().toString(Qt::ISODate);
        } else {
            result = identifier.toString();
        }
    }
    return result;
}

QString ComicProviderKross::identifier() const
{
    return pluginName() + QLatin1Char(':') + identifierToString(m_wrapper.identifierVariant());
}

QString ComicProviderKross::nextIdentifier() const
{
    return identifierToString(m_wrapper.nextIdentifierVariant());
}

QString ComicProviderKross::previousIdentifier() const
{
    return identifierToString(m_wrapper.previousIdentifierVariant());
}

QString ComicProviderKross::firstStripIdentifier() const
{
    return identifierToString(m_wrapper.firstIdentifierVariant());
}

QString ComicProviderKross::stripTitle() const
{
    return m_wrapper.title();
}

QString ComicProviderKross::additionalText() const
{
    return m_wrapper.additionalText();
}

void ComicProviderKross::pageRetrieved(int id, const QByteArray &data)
{
    m_wrapper.pageRetrieved(id, data);
}

void ComicProviderKross::pageError(int id, const QString &message)
{
    m_wrapper.pageError(id, message);
}

void ComicProviderKross::redirected(int id, const QUrl &newUrl)
{
    m_wrapper.redirected(id, newUrl);
}
