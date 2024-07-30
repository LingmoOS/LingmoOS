/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "content.h"

#include <QDateTime>

using namespace Qt::StringLiterals;
using namespace Attica;

class Q_DECL_HIDDEN Content::Private : public QSharedData
{
public:
    QString m_id;
    QString m_name;
    int m_downloads;
    int m_numberOfComments;
    int m_rating;
    QDateTime m_created;
    QDateTime m_updated;
    QList<Icon> m_icons;
    QList<QUrl> m_videos;
    QStringList m_tags;

    QMap<QString, QString> m_extendedAttributes;

    Private()
        : m_downloads(0)
        , m_numberOfComments(0)
        , m_rating(0)
    {
    }
};

Content::Content()
    : d(new Private)
{
}

Content::Content(const Attica::Content &other)
    : d(other.d)
{
}

Content &Content::operator=(const Attica::Content &other)
{
    d = other.d;
    return *this;
}

Content::~Content()
{
}

void Content::setId(const QString &u)
{
    d->m_id = u;
}

QString Content::id() const
{
    return d->m_id;
}

void Content::setName(const QString &name)
{
    d->m_name = name;
}

QString Content::name() const
{
    return d->m_name;
}

void Content::setRating(int v)
{
    d->m_rating = v;
}

int Content::rating() const
{
    return d->m_rating;
}

void Content::setDownloads(int v)
{
    d->m_downloads = v;
}

int Content::downloads() const
{
    return d->m_downloads;
}

void Content::setNumberOfComments(int v)
{
    d->m_numberOfComments = v;
}

int Content::numberOfComments() const
{
    return d->m_numberOfComments;
}

void Content::setCreated(const QDateTime &date)
{
    d->m_created = date;
}

QDateTime Content::created() const
{
    return d->m_created;
}

void Content::setUpdated(const QDateTime &date)
{
    d->m_updated = date;
}

QDateTime Content::updated() const
{
    return d->m_updated;
}

void Content::addAttribute(const QString &key, const QString &value)
{
    d->m_extendedAttributes.insert(key, value);
}

QString Content::attribute(const QString &key) const
{
    return d->m_extendedAttributes.value(key);
}

QMap<QString, QString> Content::attributes() const
{
    return d->m_extendedAttributes;
}

bool Content::isValid() const
{
    return !(d->m_id.isEmpty());
}

QString Content::summary() const
{
    return attribute(QStringLiteral("summary"));
}

QString Content::description() const
{
    return attribute(QStringLiteral("description"));
}

QUrl Content::detailpage() const
{
    return QUrl(attribute(QStringLiteral("detailpage")));
}

QString Attica::Content::changelog() const
{
    return attribute(QStringLiteral("changelog"));
}

QString Attica::Content::depend() const
{
    return attribute(QStringLiteral("depend"));
}

QList<Attica::DownloadDescription> Attica::Content::downloadUrlDescriptions() const
{
    QList<Attica::DownloadDescription> descriptions;
    QMap<QString, QString>::const_iterator iter = d->m_extendedAttributes.constBegin();
    while (iter != d->m_extendedAttributes.constEnd()) {
        const QString &key = iter.key();
        static const QLatin1String tag("downloadname");
        if (key.startsWith(tag)) {
            bool ok;
            // remove "downloadlink", get the rest as number
            const int num = QStringView(key).right(key.size() - tag.size()).toInt(&ok);
            if (ok) {
                // check if the download actually has a name
                if (!iter.value().isEmpty()) {
                    descriptions.append(downloadUrlDescription(num));
                }
            }
        }
        ++iter;
    }
    return descriptions;
}

Attica::DownloadDescription Attica::Content::downloadUrlDescription(int number) const
{
    QString num(QString::number(number));
    DownloadDescription desc;

    Attica::DownloadDescription::Type downloadType = Attica::DownloadDescription::LinkDownload;
    if (attribute(QLatin1String("downloadway") + num) == QLatin1Char('0')) {
        downloadType = Attica::DownloadDescription::FileDownload;
    } else if (attribute(QLatin1String("downloadway") + num) == QLatin1Char('1')) {
        downloadType = Attica::DownloadDescription::LinkDownload;
    } else if (attribute(QLatin1String("downloadway") + num) == QLatin1Char('2')) {
        downloadType = Attica::DownloadDescription::PackageDownload;
    }
    desc.setType(downloadType);
    desc.setId(number);
    desc.setName(attribute(QLatin1String("downloadname") + num));
    desc.setDistributionType(attribute(QLatin1String("downloadtype") + num));
    desc.setHasPrice(attribute(QLatin1String("downloadbuy") + num) == QLatin1Char('1'));
    desc.setLink(attribute(QLatin1String("downloadlink") + num));
    desc.setPriceReason(attribute(QLatin1String("downloadreason") + num));
    desc.setPriceAmount(attribute(QLatin1String("downloadprice") + num));
    desc.setSize(attribute(QLatin1String("downloadsize") + num).toUInt());
    desc.setGpgFingerprint(attribute(QLatin1String("downloadgpgfingerprint") + num));
    desc.setGpgSignature(attribute(QLatin1String("downloadgpgsignature") + num));
    desc.setPackageName(attribute(QLatin1String("downloadpackagename") + num));
    desc.setRepository(attribute(QLatin1String("downloadrepository") + num));
    desc.setTags(attribute(QLatin1String("downloadtags") + num).split(QLatin1Char(',')));
    desc.setVersion(attribute("download_version"_L1 + num));
    return desc;
}

QList<HomePageEntry> Attica::Content::homePageEntries()
{
    QList<Attica::HomePageEntry> homepages;
    QMap<QString, QString>::const_iterator iter = d->m_extendedAttributes.constBegin();
    while (iter != d->m_extendedAttributes.constEnd()) {
        QString key = iter.key();
        if (key.startsWith(QLatin1String("homepagetype"))) {
            bool ok;
            // remove "homepage", get the rest as number
            const int num = QStringView(key).right(key.size() - 12).toInt(&ok);
            if (ok) {
                // check if the homepage actually has a valid type
                if (!iter.value().isEmpty()) {
                    homepages.append(homePageEntry(num));
                }
            }
        }
        ++iter;
    }

    return homepages;
}

Attica::HomePageEntry Attica::Content::homePageEntry(int number) const
{
    QString num(QString::number(number));
    HomePageEntry homepage;

    if (number == 1 && attribute(QStringLiteral("homepage1")).isEmpty()) {
        num.clear();
    }
    homepage.setType(attribute(QLatin1String("homepagetype") + num));
    homepage.setUrl(QUrl(attribute(QLatin1String("homepage") + num)));
    return homepage;
}

QString Attica::Content::version() const
{
    return attribute(QStringLiteral("version"));
}

QString Attica::Content::author() const
{
    return attribute(QStringLiteral("personid"));
}

QString Attica::Content::license() const
{
    return attribute(QStringLiteral("licensetype"));
}

QString Attica::Content::licenseName() const
{
    return attribute(QStringLiteral("license"));
}

QString Attica::Content::previewPicture(const QString &number) const
{
    return attribute(QLatin1String("previewpic") + number);
}

QString Attica::Content::smallPreviewPicture(const QString &number) const
{
    return attribute(QLatin1String("smallpreviewpic") + number);
}

QList<Icon> Attica::Content::icons()
{
    return d->m_icons;
}

QList<Icon> Attica::Content::icons() const
{
    return d->m_icons;
}

void Attica::Content::setIcons(QList<Icon> icons)
{
    d->m_icons = std::move(icons); // TODO KF6 Make QList const & and remove the std::move
}

QList<QUrl> Attica::Content::videos()
{
    return d->m_videos;
}

void Attica::Content::setVideos(QList<QUrl> videos)
{
    d->m_videos = std::move(videos);
}

QStringList Attica::Content::tags() const
{
    return d->m_tags;
}

void Attica::Content::setTags(const QStringList &tags)
{
    d->m_tags = tags;
}
