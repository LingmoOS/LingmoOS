/*
    SPDX-FileCopyrightText: 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
    SPDX-FileCopyrightText: 2012 Marco Gulino <marco.gulino@xpeppers.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "opera.h"
#include "bookmarksrunner_defs.h"
#include "favicon.h"
#include <QDebug>
#include <QDir>
#include <QFile>

Opera::Opera(QObject *parent)
    : QObject(parent)
    , m_favicon(new FallbackFavicon(this))
{
}

QList<BookmarkMatch> Opera::match(const QString &term, bool addEverything)
{
    QList<BookmarkMatch> matches;

    QLatin1String nameStart("\tNAME=");
    QLatin1String urlStart("\tURL=");
    QLatin1String descriptionStart("\tDESCRIPTION=");

    // search
    for (const QString &entry : qAsConst(m_operaBookmarkEntries)) {
        QStringList entryLines = entry.split(QStringLiteral("\n"));
        if (!entryLines.first().startsWith(QLatin1String("#URL"))) {
            continue; // skip folder entries
        }
        entryLines.pop_front();

        QString name;
        QString url;
        QString description;

        for (const QString &line : qAsConst(entryLines)) {
            if (line.startsWith(nameStart)) {
                name = line.mid(QString(nameStart).length()).simplified();
            } else if (line.startsWith(urlStart)) {
                url = line.mid(QString(urlStart).length()).simplified();
            } else if (line.startsWith(descriptionStart)) {
                description = line.mid(QString(descriptionStart).length()).simplified();
            }
        }

        BookmarkMatch bookmarkMatch(m_favicon->iconFor(url), term, name, url, description);
        bookmarkMatch.addTo(matches, addEverything);
    }
    return matches;
}

void Opera::prepare()
{
    // open bookmarks file
    QString operaBookmarksFilePath = QDir::homePath() + "/.opera/bookmarks.adr";
    QFile operaBookmarksFile(operaBookmarksFilePath);
    if (!operaBookmarksFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // qDebug() << "Could not open Operas Bookmark File " + operaBookmarksFilePath;
        return;
    }

    // check format
    QString firstLine = operaBookmarksFile.readLine();
    if (firstLine.compare(QLatin1String("Opera Hotlist version 2.0\n"))) {
        // qDebug() << "Format of Opera Bookmarks File might have changed.";
    }
    operaBookmarksFile.readLine(); // skip options line ("Options: encoding = utf8, version=3")
    operaBookmarksFile.readLine(); // skip empty line

    // load contents
    QString contents = operaBookmarksFile.readAll();
    m_operaBookmarkEntries = contents.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);

    // close file
    operaBookmarksFile.close();
}

void Opera::teardown()
{
    m_operaBookmarkEntries.clear();
}
