/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef POSTFILEDATA_H
#define POSTFILEDATA_H

#include <memory>

#include <QByteArray>
#include <QIODevice>
#include <QNetworkRequest>

namespace Attica
{
class PostFileDataPrivate;

class PostFileData
{
public:
    /**
     * Prepare a QNetworkRequest and QByteArray for sending a HTTP POST.
     * Parameters and files can be added with addArgument() and addFile()
     * Do not add anything after calling request or data for the first time.
     */
    PostFileData(const QUrl &url);
    ~PostFileData();

    void addArgument(const QString &key, const QString &value);
    void addFile(const QString &fileName, QIODevice *file, const QString &mimeType);
    void addFile(const QString &fileName, const QByteArray &file, const QString &mimeType, const QString &fieldName = QStringLiteral("localfile"));

    QNetworkRequest request();
    QByteArray data();

private:
    void finish();
    QString randomString(int length);
    std::unique_ptr<PostFileDataPrivate> d;
    Q_DISABLE_COPY(PostFileData)
};

}

#endif // POSTFILEDATA_H
