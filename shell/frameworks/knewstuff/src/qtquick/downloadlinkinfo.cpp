/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "downloadlinkinfo.h"
#include <KFormat>
#include <QMimeDatabase>

class DownloadLinkInfoPrivate : public QSharedData
{
public:
    QString name;
    QString priceAmount;
    QString distributionType;
    QString descriptionLink;
    int id = 0;
    bool isDownloadtypeLink = true;
    quint64 size = 0;
    QString mimeType;
    QString icon;
};

DownloadLinkInfo::DownloadLinkInfo(const KNSCore::Entry::DownloadLinkInformation &data)
    : d(new DownloadLinkInfoPrivate)
{
    d->name = data.name;
    d->priceAmount = data.priceAmount;
    d->distributionType = data.distributionType;
    d->descriptionLink = data.descriptionLink;
    d->id = data.id;
    d->isDownloadtypeLink = data.isDownloadtypeLink;
    d->size = data.size;
    QMimeDatabase db;
    for (QString string : data.tags) {
        if (string.startsWith(QStringLiteral("data##mimetype="))) {
            d->mimeType = string.split(QStringLiteral("=")).last();
        }
    }
    d->icon = db.mimeTypeForName(d->mimeType).iconName();
    if (d->icon.isEmpty()) {
        d->icon = db.mimeTypeForName(d->mimeType).genericIconName();
    }
    if (d->icon.isEmpty()) {
        d->icon = QStringLiteral("download");
    }
}

DownloadLinkInfo::DownloadLinkInfo(const DownloadLinkInfo &) = default;
DownloadLinkInfo &DownloadLinkInfo::operator=(const DownloadLinkInfo &) = default;
DownloadLinkInfo::~DownloadLinkInfo() = default;

QString DownloadLinkInfo::name() const
{
    return d->name;
}

QString DownloadLinkInfo::priceAmount() const
{
    return d->priceAmount;
}

QString DownloadLinkInfo::distributionType() const
{
    return d->distributionType;
}

QString DownloadLinkInfo::descriptionLink() const
{
    return d->descriptionLink;
}

int DownloadLinkInfo::id() const
{
    return d->id;
}

bool DownloadLinkInfo::isDownloadtypeLink() const
{
    return d->isDownloadtypeLink;
}

quint64 DownloadLinkInfo::size() const
{
    return d->size;
}

QString DownloadLinkInfo::formattedSize() const
{
    static const KFormat formatter;
    if (d->size == 0) {
        return QString();
    }
    return formatter.formatByteSize(d->size * 1000);
}

QString DownloadLinkInfo::icon() const
{
    return d->icon;
}

#include "moc_downloadlinkinfo.cpp"
