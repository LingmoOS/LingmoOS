/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 1999 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <kde@jowenn.at>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "postfiledata.h"

#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>

namespace Attica
{
class PostFileDataPrivate
{
public:
    QByteArray buffer;
    QByteArray boundary;
    QUrl url;
    bool finished = false;
};

PostFileData::PostFileData(const QUrl &url)
    : d(std::make_unique<PostFileDataPrivate>())
{
    d->url = url;
    d->boundary = "----------" + randomString(42 + 13).toLatin1();
}

PostFileData::~PostFileData() = default;

QString PostFileData::randomString(int length)
{
    if (length <= 0) {
        return QString();
    }

    QString str;
    str.resize(length);
    int i = 0;
    auto *generator = QRandomGenerator::global();
    while (length--) {
        int r = generator->bounded(62);
        r += 48;
        if (r > 57) {
            r += 7;
        }
        if (r > 90) {
            r += 6;
        }
        str[i++] = QLatin1Char(char(r));
    }
    return str;
}

void PostFileData::addArgument(const QString &key, const QString &value)
{
    if (d->finished) {
        qWarning() << "PostFileData::addFile: should not add data after calling request() or data()";
    }
    QByteArray data(
        "--" + d->boundary + "\r\n"
        "Content-Disposition: form-data; name=\"" + key.toLatin1() +
        "\"\r\n\r\n" + value.toUtf8() + "\r\n");

    d->buffer.append(data);
}

/*
void PostFileData::addFile(const QString& fileName, QIODevice* file, const QString& mimeType)
{
    if (d->finished) {
        qCDebug(ATTICA) << "PostFileData::addFile: should not add data after calling request() or data()";
    }
    QByteArray data = file->readAll();
    addFile(fileName, data, mimeType);
}
*/

void PostFileData::addFile(const QString &fileName, const QByteArray &file, const QString &mimeType, const QString &fieldName)
{
    if (d->finished) {
        qWarning() << "PostFileData::addFile: should not add data after calling request() or data()";
    }

    QByteArray data(
        "--" + d->boundary + "\r\n"
        "Content-Disposition: form-data; name=\"");
    data.append(fieldName.toLatin1());
    data.append("\"; filename=\"" + fileName.toUtf8() + "\"\r\nContent-Type: " + mimeType.toLatin1() + "\r\n\r\n");

    d->buffer.append(data);
    d->buffer.append(file + QByteArray("\r\n"));
}

QNetworkRequest PostFileData::request()
{
    if (!d->finished) {
        finish();
    }
    QNetworkRequest request;
    request.setUrl(d->url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("multipart/form-data; boundary=" + d->boundary));
    request.setHeader(QNetworkRequest::ContentLengthHeader, d->buffer.length());
    return request;
}

QByteArray PostFileData::data()
{
    if (!d->finished) {
        finish();
    }
    return d->buffer;
}

void PostFileData::finish()
{
    Q_ASSERT(!d->finished);
    d->finished = true;
    d->buffer.append("--" + d->boundary + "--");
}

}
