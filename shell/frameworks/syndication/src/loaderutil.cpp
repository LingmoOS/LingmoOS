/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "loaderutil_p.h"
#include <QDebug>
#include <QRegularExpression>

// #define DEBUG_PARSING_FEED
#ifdef DEBUG_PARSING_FEED
#include <QFile>
#include <QTextStream>
#endif
QUrl Syndication::LoaderUtil::parseFeed(const QByteArray &data, const QUrl &url)
{
#ifdef DEBUG_PARSING_FEED
    qDebug() << " QUrl Syndication::LoaderUtil::parseFeed(const QByteArray &data, const QUrl &url)";
    QFile headerFile(QStringLiteral("/tmp/bb.txt"));
    headerFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outHeaderStream(&headerFile);
    outHeaderStream << data;
    headerFile.close();
#endif
    QUrl discoveredFeedURL;
    QString str = QString::fromLatin1(data.constData()).simplified();
    QString s2;
    // QTextStream ts( &str, QIODevice::WriteOnly );
    // ts << data.data();

    // "<[\\s]link[^>]*rel[\\s]=[\\s]\\\"[\\s]alternate[\\s]\\\"[^>]*>"
    // "type[\\s]=[\\s]\\\"application/rss+xml\\\""
    // "href[\\s]=[\\s]\\\"application/rss+xml\\\""

    // test regexp: https://www.regexplanet.com/advanced/perl/index.html

    const static QRegularExpression rx0(
        QStringLiteral(
            R"((?:REL)[^=]*=[^sAa]*(?:service.feed|ALTERNATE)[^sAa]*[\s]*type[^=]*="application/rss\+xml"[^s]*[\s]*[^s]*(?:HREF)[^=]*?=[^A-Z0-9-_~,./$]*([^'\">\s]*))"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match;
    if ((match = rx0.match(str)).hasMatch()) {
        s2 = match.captured(1);
    } else {
        const static QRegularExpression rx(
            QStringLiteral(
                R"((?:REL)[^=]*=[^sAa]*(?:service.feed|ALTERNATE)[^sAa]*[\s]*type[^=]*=\"application/rss\+xml\"[^s][^s](?:[^>]*)[\s]*[^s]*(?:HREF)[^=]*=[^A-Z0-9-_~,./$]*([^'\">\s]*))"),
            QRegularExpression::CaseInsensitiveOption);
        if ((match = rx.match(str)).hasMatch()) {
            s2 = match.captured(1);
        } else {
            static const QRegularExpression rx2(
                QStringLiteral(R"((?:REL)[^=]*=[^sAa]*(?:service.feed|ALTERNATE)[\s]*[^s][^s](?:[^>]*)(?:HREF)[^=]*=[^A-Z0-9-_~,./$]*([^'\">\s]*))"),
                QRegularExpression::CaseInsensitiveOption);
            if ((match = rx2.match(str)).hasMatch()) {
                s2 = match.captured(1);
            } else {
                // does not support Atom/RSS autodiscovery.. try finding feeds by brute force....
                QStringList feeds;
                QString host = url.host();
                static const QRegularExpression rx3(QStringLiteral(R"((?:<A )[^H]*(?:HREF)[^=]*=[^A-Z0-9-_~,./]*([^'\">\s]*))"));
                QRegularExpressionMatchIterator iter = rx3.globalMatch(str);
                while (iter.hasNext()) {
                    match = iter.next();
                    s2 = match.captured(1);
                    if (s2.endsWith(QLatin1String(".rdf")) //
                        || s2.endsWith(QLatin1String(".rss")) //
                        || s2.endsWith(QLatin1String(".xml"))) {
                        feeds.append(s2);
                    }
                }

                // Prefer feeds on same host
                auto it = std::find_if(feeds.cbegin(), feeds.cend(), [&host](const QString &s) {
                    return QUrl(s).host() == host;
                });
                if (it != feeds.cend()) {
                    s2 = *it;
                }
            }
        }
    }

    if (s2.isNull()) {
        return discoveredFeedURL;
    }

    if (QUrl(s2).isRelative()) {
        if (s2.startsWith(QLatin1String("//"))) {
            s2.prepend(url.scheme() + QLatin1Char(':'));
            discoveredFeedURL = QUrl(s2);
        } else if (s2.startsWith(QLatin1Char('/'))) {
            discoveredFeedURL = url;
            discoveredFeedURL.setPath(s2);
        } else {
            discoveredFeedURL = url;
            discoveredFeedURL.setPath(discoveredFeedURL.path() + QLatin1Char('/') + s2);
        }
    } else {
        discoveredFeedURL = QUrl(s2);
    }

    return discoveredFeedURL;
}
