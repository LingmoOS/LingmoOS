/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfileutils.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QRegularExpression>

#include <set>

QString KFileUtils::makeSuggestedName(const QString &oldName)
{
    QString basename;

    // Extract the original file extension from the filename
    QMimeDatabase db;
    QString nameSuffix = db.suffixForFileName(oldName);

    if (oldName.lastIndexOf(QLatin1Char('.')) == 0) {
        basename = QStringLiteral(".");
        nameSuffix = oldName;
    } else if (nameSuffix.isEmpty()) {
        const int lastDot = oldName.lastIndexOf(QLatin1Char('.'));
        if (lastDot == -1) {
            basename = oldName;
        } else {
            basename = oldName.left(lastDot);
            nameSuffix = oldName.mid(lastDot);
        }
    } else {
        nameSuffix.prepend(QLatin1Char('.'));
        basename = oldName.left(oldName.length() - nameSuffix.length());
    }

    // check if (number) exists at the end of the oldName and increment that number
    const static QRegularExpression re(QStringLiteral("\\((\\d+)\\)"));
    QRegularExpressionMatch rmatch;
    if (oldName.lastIndexOf(re, -1, &rmatch) != -1) {
        const int currentNum = rmatch.captured(1).toInt();
        const QString number = QString::number(currentNum + 1);
        basename.replace(rmatch.capturedStart(1), rmatch.capturedLength(1), number);
    } else {
        // number does not exist, so just append " (1)" to filename
        basename += QLatin1String(" (1)");
    }

    return basename + nameSuffix;
}

QString KFileUtils::suggestName(const QUrl &baseURL, const QString &oldName)
{
    QString suggestedName = makeSuggestedName(oldName);

    if (baseURL.isLocalFile()) {
        const QString basePath = baseURL.toLocalFile() + QLatin1Char('/');
        while (QFileInfo::exists(basePath + suggestedName)) {
            suggestedName = makeSuggestedName(suggestedName);
        }
    }

    return suggestedName;
}

QStringList KFileUtils::findAllUniqueFiles(const QStringList &dirs, const QStringList &nameFilters)
{
    QStringList foundFilePaths;
    std::set<QString> foundFileNames;
    for (const QString &dir : dirs) {
        QDirIterator it(dir, nameFilters, QDir::Files);
        while (it.hasNext()) {
            it.next();
            const auto [iter, isFirstSeen] = foundFileNames.insert(it.fileName());
            if (isFirstSeen) {
                foundFilePaths << it.filePath();
            }
        }
    }
    return foundFilePaths;
}
