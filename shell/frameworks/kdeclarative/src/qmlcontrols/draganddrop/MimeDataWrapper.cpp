/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "MimeDataWrapper.h"
#include <QMimeData>
#include <QUrl>

MimeDataWrapper::MimeDataWrapper(const QMimeData *data, QObject *parent)
    : QObject(parent)
    , m_data(data)
{
}

QString MimeDataWrapper::text() const
{
    return m_data->text();
}

QString MimeDataWrapper::html() const
{
    return m_data->html();
}

QUrl MimeDataWrapper::url() const
{
    if (m_data->hasUrls() && !m_data->urls().isEmpty()) {
        return m_data->urls().constFirst();
    }
    return QUrl();
}

bool MimeDataWrapper::hasUrls() const
{
    return m_data->hasUrls();
}

QJsonArray MimeDataWrapper::urls() const
{
    QJsonArray varUrls;
    const auto urls = m_data->urls();
    for (const QUrl &url : urls) {
        varUrls.append(url.toString());
    }
    return varUrls;
}

QVariant MimeDataWrapper::color() const
{
    if (m_data->hasColor()) {
        return m_data->colorData();
    } else {
        return QVariant();
    }
}

QStringList MimeDataWrapper::formats() const
{
    return m_data->formats();
}

QByteArray MimeDataWrapper::getDataAsByteArray(const QString &format)
{
    return m_data->data(format);
}

QVariant MimeDataWrapper::source() const
{
    //     In case it comes from a DeclarativeMimeData
    return m_data->property("source");
}

QMimeData *MimeDataWrapper::mimeData() const
{
    return const_cast<QMimeData *>(m_data);
}

#include "moc_MimeDataWrapper.cpp"
