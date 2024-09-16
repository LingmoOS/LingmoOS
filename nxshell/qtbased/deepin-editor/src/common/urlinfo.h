// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URLINFO_H
#define URLINFO_H

#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QDir>
#include <QDebug>

class UrlInfo
{
public:
    UrlInfo(QString path)
    {
        // Just check if the path is an existing file.
        if (QFile::exists(path)) {
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
            return;
        }

        const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(path);

        if (match.isValid()) {
            // cut away line/column specification from the path.
            path.chop(match.capturedLength());
        }

        // make relative paths absolute using the current working directory
        // prefer local file, if in doubt!
        url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);

        // in some cases, this will fail, e.g.
        // assume a local file and just convert it to an url.
        if (!url.isValid()) {
            // create absolute file path, we will e.g. pass this over dbus to other processes
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
        }
    }

    QUrl url;
};

#endif // URLINFO_H
