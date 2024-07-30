/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "downloaddescription.h"

#include <QStringList>

namespace Attica
{
class Q_DECL_HIDDEN DownloadDescription::Private : public QSharedData
{
public:
    int id = 0;
    Attica::DownloadDescription::Type type = Attica::DownloadDescription::FileDownload;
    bool hasPrice = false;
    QString category;
    QString name;
    QString link;
    QString distributionType;
    QString priceReason;
    QString priceAmount;
    QString gpgFingerprint;
    QString gpgSignature;
    QString packageName;
    QString repository;
    uint size = 0;
    QStringList tags;
    QString version;
};
}

using namespace Attica;

DownloadDescription::DownloadDescription()
    : d(new Private)
{
}

DownloadDescription::DownloadDescription(const Attica::DownloadDescription &other)
    : d(other.d)
{
}

DownloadDescription &DownloadDescription::operator=(const Attica::DownloadDescription &other)
{
    d = other.d;
    return *this;
}

DownloadDescription::~DownloadDescription()
{
}

QString Attica::DownloadDescription::category() const
{
    return d->category;
}

int DownloadDescription::id() const
{
    return d->id;
}

void DownloadDescription::setId(int id)
{
    d->id = id;
}

void DownloadDescription::setCategory(const QString &category)
{
    d->category = category;
}

QString Attica::DownloadDescription::distributionType() const
{
    return d->distributionType;
}

void DownloadDescription::setDistributionType(const QString &distributionType)
{
    d->distributionType = distributionType;
}

bool Attica::DownloadDescription::hasPrice() const
{
    return d->hasPrice;
}

void DownloadDescription::setHasPrice(bool hasPrice)
{
    d->hasPrice = hasPrice;
}

Attica::DownloadDescription::Type DownloadDescription::type() const
{
    return d->type;
}

void DownloadDescription::setType(Attica::DownloadDescription::Type type)
{
    d->type = type;
}

QString Attica::DownloadDescription::link() const
{
    return d->link;
}

void DownloadDescription::setLink(const QString &link)
{
    d->link = link;
}

QString Attica::DownloadDescription::name() const
{
    return d->name;
}

void DownloadDescription::setName(const QString &name)
{
    d->name = name;
}

QString Attica::DownloadDescription::priceAmount() const
{
    return d->priceAmount;
}

void DownloadDescription::setPriceAmount(const QString &priceAmount)
{
    d->priceAmount = priceAmount;
}

QString Attica::DownloadDescription::priceReason() const
{
    return d->priceReason;
}

void Attica::DownloadDescription::setPriceReason(const QString &priceReason)
{
    d->priceReason = priceReason;
}

uint Attica::DownloadDescription::size() const
{
    return d->size;
}

void Attica::DownloadDescription::setSize(uint size)
{
    d->size = size;
}

QString Attica::DownloadDescription::gpgFingerprint() const
{
    return d->gpgFingerprint;
}

void Attica::DownloadDescription::setGpgFingerprint(const QString &fingerprint)
{
    d->gpgFingerprint = fingerprint;
}

QString Attica::DownloadDescription::gpgSignature() const
{
    return d->gpgSignature;
}

void Attica::DownloadDescription::setGpgSignature(const QString &signature)
{
    d->gpgSignature = signature;
}

QString Attica::DownloadDescription::packageName() const
{
    return d->packageName;
}

void Attica::DownloadDescription::setPackageName(const QString &packageName)
{
    d->packageName = packageName;
}

QString Attica::DownloadDescription::repository() const
{
    return d->repository;
}

void Attica::DownloadDescription::setRepository(const QString &repository)
{
    d->repository = repository;
}

QStringList Attica::DownloadDescription::tags() const
{
    return d->tags;
}

void Attica::DownloadDescription::setTags(const QStringList &tags)
{
    d->tags = tags;
}

QString Attica::DownloadDescription::version() const
{
    return d->version;
}

void Attica::DownloadDescription::setVersion(const QString &version)
{
    d->version = version;
}
