// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <QFile>

#include <sentryconnection.h>

class FileReply : public SentryReply
{
public:
    explicit FileReply(const QString &path, QObject *parent = nullptr)
        : SentryReply(parent)
        , m_file(path)
    {
        const auto open = m_file.open(QFile::ReadOnly);
        QMetaObject::invokeMethod(this, &FileReply::finished, Qt::QueuedConnection);
        Q_ASSERT(open);
    }

    QByteArray readAll() override
    {
        return m_file.readAll();
    }

    QNetworkReply::NetworkError error() override
    {
        return m_error;
    }

    QString errorString() override
    {
        return m_errorString;
    }

    QNetworkReply::NetworkError m_error = QNetworkReply::NoError;
    QString m_errorString;
    QFile m_file;
};
