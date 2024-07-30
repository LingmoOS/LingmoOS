/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "downloaditem.h"

using namespace Attica;

class Q_DECL_HIDDEN DownloadItem::Private : public QSharedData
{
public:
    QUrl m_url;
    QString m_mimeType;
    QString m_packageName;
    QString m_packageRepository;
    QString m_gpgFingerprint;
    QString m_gpgSignature;
    Attica::DownloadDescription::Type m_type;

    Private()
        : m_type(DownloadDescription::FileDownload)
    {
    }
};

DownloadItem::DownloadItem()
    : d(new Private)
{
}

DownloadItem::DownloadItem(const Attica::DownloadItem &other)
    : d(other.d)
{
}

DownloadItem &DownloadItem::operator=(const Attica::DownloadItem &other)
{
    d = other.d;
    return *this;
}

DownloadItem::~DownloadItem()
{
}

void DownloadItem::setUrl(const QUrl &url)
{
    d->m_url = url;
}

QUrl DownloadItem::url() const
{
    return d->m_url;
}

void DownloadItem::setMimeType(const QString &mimeType)
{
    d->m_mimeType = mimeType;
}

QString DownloadItem::mimeType() const
{
    return d->m_mimeType;
}

void DownloadItem::setPackageName(const QString &packageName)
{
    d->m_packageName = packageName;
}

QString DownloadItem::packageName() const
{
    return d->m_packageName;
}

void DownloadItem::setPackageRepository(const QString &packageRepository)
{
    d->m_packageRepository = packageRepository;
}

QString DownloadItem::packageRepository() const
{
    return d->m_packageRepository;
}

void DownloadItem::setGpgFingerprint(const QString &gpgFingerprint)
{
    d->m_gpgFingerprint = gpgFingerprint;
}

QString DownloadItem::gpgFingerprint() const
{
    return d->m_gpgFingerprint;
}

void DownloadItem::setGpgSignature(const QString &gpgSignature)
{
    d->m_gpgSignature = gpgSignature;
}

QString DownloadItem::gpgSignature() const
{
    return d->m_gpgSignature;
}

void DownloadItem::setType(Attica::DownloadDescription::Type type)
{
    d->m_type = type;
}

Attica::DownloadDescription::Type DownloadItem::type()
{
    return d->m_type;
}
