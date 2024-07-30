/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef URLINFO_H
#define URLINFO_H

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QUrl>

/**
 * Represents a file to be opened, consisting of its URL and the cursor to jump to.
 */
class UrlInfo
{
public:
    /**
     * Parses an argument and determines its line number and column and full path
     * @param pathOrUrl path passed on e.g. command line to parse into an URL or just an URL
     */
    UrlInfo(const QString &pathOrUrl)
        : line(0)
        , column(0)
    {
        /**
         * first try: just check if the path is an existing file
         */
        if (QFile::exists(pathOrUrl)) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             * and then we are done, no cursor can be detected here!
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(pathOrUrl));
            return;
        }

        if (auto inputUrl = QUrl::fromUserInput(pathOrUrl); inputUrl.isLocalFile() && QFile::exists(inputUrl.toLocalFile())) {
            url = inputUrl;
            return;
        }

        /**
         * if the path starts with http:// or any other scheme, except file://
         * we also don't want to do anything with URL
         */
        if (!QUrl::fromUserInput(pathOrUrl).isLocalFile()) {
            url = QUrl::fromUserInput(pathOrUrl, QDir::currentPath(), QUrl::DefaultResolution);
            // relative paths are not isLocalFile(), but not valid too, so we don't want them
            if (url.isValid()) {
                return;
            }
        }

        /**
         * ok, the path as is, is no existing file, now, cut away :xx:yy stuff as cursor
         * this will make test:50 to test with line 50
         */
        QString pathOrUrl2 = pathOrUrl;
        const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(pathOrUrl2);
        if (match.isValid()) {
            /**
             * cut away the line/column specification from the path
             */
            pathOrUrl2.chop(match.capturedLength());

            /**
             * set right cursor position
             */
            line = match.capturedView(1).toUInt();
            column = match.capturedView(2).toUInt();
        }

        /**
         * construct url:
         *   - make relative paths absolute using the current working directory
         *   - do not prefer local file, to be able to open things like foo.com in browser
         */
        url = QUrl::fromUserInput(pathOrUrl2, QDir::currentPath(), QUrl::DefaultResolution);

        /**
         * in some cases, this will fail, e.g. if you have line/column specs like test.c:10:1
         * => fallback: assume a local file and just convert it to an url
         */
        if (!url.isValid()) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(pathOrUrl2));
        }
    }

    bool atStart() const
    {
        return (line == 0 || line == 1) && (column == 0 || column == 1);
    }

    /**
     * url computed out of the passed path or URL
     */
    QUrl url;

    /**
     * initial cursor position, if any found inside the path as line/column specification at the end
     */
    unsigned line, column;
};

#endif // URLINFO_H
