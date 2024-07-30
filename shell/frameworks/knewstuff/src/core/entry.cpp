/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "entry.h"
#include "entry_p.h"

#include <QDomElement>
#include <QMetaEnum>
#include <QStringList>
#include <QXmlStreamReader>
#include <knewstuffcore_debug.h>

#include "xmlloader_p.h"

using namespace KNSCore;

Entry::Entry()
    : d(new EntryPrivate())
{
}

Entry::Entry(const Entry &other)
    : d(other.d)
{
}

Entry &Entry::operator=(const Entry &other)
{
    d = other.d;
    return *this;
}

bool Entry::operator<(const KNSCore::Entry &other) const
{
    return d->mUniqueId < other.d->mUniqueId;
}

bool Entry::operator==(const KNSCore::Entry &other) const
{
    return d->mUniqueId == other.d->mUniqueId && d->mProviderId == other.d->mProviderId;
}

Entry::~Entry() = default;

bool Entry::isValid() const
{
    return !d->mUniqueId.isEmpty(); // This should not use the uniqueId getter due to the fallback!
}

QString Entry::name() const
{
    return d->mName;
}

void Entry::setName(const QString &name)
{
    d->mName = name;
}

QString Entry::uniqueId() const
{
    return d->mUniqueId.isEmpty() ? d->mRequestedUniqueId : d->mUniqueId;
}

void Entry::setUniqueId(const QString &id)
{
    d->mUniqueId = id;
}

QString Entry::providerId() const
{
    return d->mProviderId;
}

void Entry::setProviderId(const QString &id)
{
    d->mProviderId = id;
}

QStringList KNSCore::Entry::tags() const
{
    return d->mTags;
}

void KNSCore::Entry::setTags(const QStringList &tags)
{
    d->mTags = tags;
}

QString Entry::category() const
{
    return d->mCategory;
}

void Entry::setCategory(const QString &category)
{
    d->mCategory = category;
}

QUrl Entry::homepage() const
{
    return d->mHomepage;
}

void Entry::setHomepage(const QUrl &page)
{
    d->mHomepage = page;
}

Author Entry::author() const
{
    return d->mAuthor;
}

void Entry::setAuthor(const KNSCore::Author &author)
{
    d->mAuthor = author;
}

QString Entry::license() const
{
    return d->mLicense;
}

void Entry::setLicense(const QString &license)
{
    d->mLicense = license;
}

QString Entry::summary() const
{
    return d->mSummary;
}

void Entry::setSummary(const QString &summary)
{
    d->mSummary = summary;
}

QString Entry::shortSummary() const
{
    return d->mShortSummary;
}

void Entry::setShortSummary(const QString &summary)
{
    d->mShortSummary = summary;
}

void Entry::setChangelog(const QString &changelog)
{
    d->mChangelog = changelog;
}

QString Entry::changelog() const
{
    return d->mChangelog;
}

QString Entry::version() const
{
    return d->mVersion;
}

void Entry::setVersion(const QString &version)
{
    d->mVersion = version;
}

QDate Entry::releaseDate() const
{
    return d->mReleaseDate;
}

void Entry::setReleaseDate(const QDate &releasedate)
{
    d->mReleaseDate = releasedate;
}

QString Entry::payload() const
{
    return d->mPayload;
}

void Entry::setPayload(const QString &url)
{
    d->mPayload = url;
}

QDate Entry::updateReleaseDate() const
{
    return d->mUpdateReleaseDate;
}

void Entry::setUpdateReleaseDate(const QDate &releasedate)
{
    d->mUpdateReleaseDate = releasedate;
}

QString Entry::updateVersion() const
{
    return d->mUpdateVersion;
}

void Entry::setUpdateVersion(const QString &version)
{
    d->mUpdateVersion = version;
}

QString Entry::previewUrl(PreviewType type) const
{
    return d->mPreviewUrl[type];
}

void Entry::setPreviewUrl(const QString &url, PreviewType type)
{
    d->mPreviewUrl[type] = url;
}

QImage Entry::previewImage(PreviewType type) const
{
    return d->mPreviewImage[type];
}

void Entry::setPreviewImage(const QImage &image, PreviewType type)
{
    d->mPreviewImage[type] = image;
}

int Entry::rating() const
{
    return d->mRating;
}

void Entry::setRating(int rating)
{
    d->mRating = rating;
}

int Entry::numberOfComments() const
{
    return d->mNumberOfComments;
}

void Entry::setNumberOfComments(int comments)
{
    d->mNumberOfComments = comments;
}

int Entry::downloadCount() const
{
    return d->mDownloadCount;
}

void Entry::setDownloadCount(int downloads)
{
    d->mDownloadCount = downloads;
}

int Entry::numberFans() const
{
    return d->mNumberFans;
}

void Entry::setNumberFans(int fans)
{
    d->mNumberFans = fans;
}

QString Entry::donationLink() const
{
    return d->mDonationLink;
}

void Entry::setDonationLink(const QString &link)
{
    d->mDonationLink = link;
}

int Entry::numberKnowledgebaseEntries() const
{
    return d->mNumberKnowledgebaseEntries;
}
void Entry::setNumberKnowledgebaseEntries(int num)
{
    d->mNumberKnowledgebaseEntries = num;
}

QString Entry::knowledgebaseLink() const
{
    return d->mKnowledgebaseLink;
}
void Entry::setKnowledgebaseLink(const QString &link)
{
    d->mKnowledgebaseLink = link;
}

Entry::Source Entry::source() const
{
    return d->mSource;
}

void Entry::setEntryType(Entry::EntryType type)
{
    d->mEntryType = type;
}

Entry::EntryType Entry::entryType() const
{
    return d->mEntryType;
}

void Entry::setSource(Source source)
{
    d->mSource = source;
}

KNSCore::Entry::Status Entry::status() const
{
    return d->mStatus;
}

void Entry::setStatus(KNSCore::Entry::Status status)
{
    d->mStatus = status;
}

void KNSCore::Entry::setInstalledFiles(const QStringList &files)
{
    d->mInstalledFiles = files;
}

QStringList KNSCore::Entry::installedFiles() const
{
    return d->mInstalledFiles;
}

QStringList KNSCore::Entry::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}

int KNSCore::Entry::downloadLinkCount() const
{
    return d->mDownloadLinkInformationList.size();
}

QList<KNSCore::Entry::DownloadLinkInformation> KNSCore::Entry::downloadLinkInformationList() const
{
    const auto infos = d->mDownloadLinkInformationList;
    QList<KNSCore::Entry::DownloadLinkInformation> ret;
    ret.reserve(infos.size());
    for (const auto &info : infos) {
        ret.append({.name = info.name,
                    .priceAmount = info.priceAmount,
                    .distributionType = info.distributionType,
                    .descriptionLink = info.descriptionLink,
                    .id = info.id,
                    .isDownloadtypeLink = info.isDownloadtypeLink,
                    .size = info.size,
                    .tags = info.tags});
    }
    return ret;
}

void KNSCore::Entry::appendDownloadLinkInformation(const KNSCore::Entry::DownloadLinkInformation &info)
{
    d->mDownloadLinkInformationList.append({.name = info.name,
                                            .priceAmount = info.priceAmount,
                                            .distributionType = info.distributionType,
                                            .descriptionLink = info.descriptionLink,
                                            .id = info.id,
                                            .isDownloadtypeLink = info.isDownloadtypeLink,
                                            .size = info.size,
                                            .tags = info.tags,
                                            .version = QString()});
}

void Entry::clearDownloadLinkInformation()
{
    d->mDownloadLinkInformationList.clear();
}

static QXmlStreamReader::TokenType readNextSkipComments(QXmlStreamReader *xml)
{
    do {
        xml->readNext();
    } while (xml->tokenType() == QXmlStreamReader::Comment || (xml->tokenType() == QXmlStreamReader::Characters && xml->text().trimmed().isEmpty()));
    return xml->tokenType();
}

static QString readText(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    QString ret;
    const auto token = readNextSkipComments(xml);
    if (token == QXmlStreamReader::Characters) {
        ret = xml->text().toString();
    }
    return ret;
}

static QString readStringTrimmed(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    QString ret = readText(xml).trimmed();

    if (xml->tokenType() == QXmlStreamReader::Characters) {
        readNextSkipComments(xml);
    }
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::EndElement);
    return ret;
}

static int readInt(QXmlStreamReader *xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    int ret = readText(xml).toInt();

    xml->readNext();
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::EndElement);
    return ret;
}

bool KNSCore::Entry::setEntryXML(QXmlStreamReader &reader)
{
    if (reader.name() != QLatin1String("stuff")) {
        qCWarning(KNEWSTUFFCORE) << "Parsing Entry from invalid XML. Reader tag name was expected to be \"stuff\", but was found as:" << reader.name();
        return false;
    }

    d->mCategory = reader.attributes().value(QStringLiteral("category")).toString();

    while (!reader.atEnd()) {
        const auto token = readNextSkipComments(&reader);
        if (token == QXmlStreamReader::EndElement) {
            break;
        } else if (token != QXmlStreamReader::StartElement) {
            continue;
        }

        if (reader.name() == QLatin1String("name")) {
            // TODO maybe do something with the language attribute? QString lang = e.attribute("lang");
            d->mName = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("author")) {
            // ### careful, the following variables are string views that become invalid when we
            // proceed with reading from reader, such as the readStringTrimmed call below does!
            const auto email = reader.attributes().value(QStringLiteral("email"));
            const auto jabber = reader.attributes().value(QStringLiteral("im"));
            const auto homepage = reader.attributes().value(QStringLiteral("homepage"));
            d->mAuthor.setEmail(email.toString());
            d->mAuthor.setJabber(jabber.toString());
            d->mAuthor.setHomepage(homepage.toString());
            d->mAuthor.setName(readStringTrimmed(&reader));
        } else if (reader.name() == QLatin1String("providerid")) {
            d->mProviderId = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("homepage")) {
            d->mHomepage = QUrl(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else if (reader.name() == QLatin1String("licence")) { // krazy:exclude=spelling
            d->mLicense = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("summary")) {
            d->mSummary = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("changelog")) {
            d->mChangelog = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("version")) {
            d->mVersion = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("releasedate")) {
            d->mReleaseDate = QDate::fromString(readStringTrimmed(&reader), Qt::ISODate);
        } else if (reader.name() == QLatin1String("preview")) {
            // TODO support for all 6 image links
            d->mPreviewUrl[PreviewSmall1] = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("previewBig")) {
            d->mPreviewUrl[PreviewBig1] = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("payload")) {
            d->mPayload = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("rating")) {
            d->mRating = readInt(&reader);
        } else if (reader.name() == QLatin1String("downloads")) {
            d->mDownloadCount = readInt(&reader);
        } else if (reader.name() == QLatin1String("category")) {
            d->mCategory = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("signature")) {
            d->mSignature = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("checksum")) {
            d->mChecksum = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("installedfile")) {
            d->mInstalledFiles.append(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else if (reader.name() == QLatin1String("id")) {
            d->mUniqueId = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("tags")) {
            d->mTags = reader.readElementText(QXmlStreamReader::SkipChildElements).split(QLatin1Char(','));
        } else if (reader.name() == QLatin1String("status")) {
            const auto statusText = readText(&reader);
            if (statusText == QLatin1String("installed")) {
                qCDebug(KNEWSTUFFCORE) << "Found an installed entry in registry";
                d->mStatus = KNSCore::Entry::Installed;
            } else if (statusText == QLatin1String("updateable")) {
                d->mStatus = KNSCore::Entry::Updateable;
            }
            if (reader.tokenType() == QXmlStreamReader::Characters) {
                readNextSkipComments(&reader);
            }
        }
        Q_ASSERT_X(reader.tokenType() == QXmlStreamReader::EndElement,
                   Q_FUNC_INFO,
                   QStringLiteral("token name was %1 and the type was %2").arg(reader.name().toString(), reader.tokenString()).toLocal8Bit().data());
    }

    // Validation
    if (d->mName.isEmpty()) {
        qWarning() << "Entry: no name given";
        return false;
    }

    if (d->mUniqueId.isEmpty()) {
        if (!d->mPayload.isEmpty()) {
            d->mUniqueId = d->mPayload;
        } else {
            d->mUniqueId = d->mName;
        }
    }

    if (d->mPayload.isEmpty()) {
        qWarning() << "Entry: no payload URL given for: " << d->mName << " - " << d->mUniqueId;
        return false;
    }
    return true;
}

bool KNSCore::Entry::setEntryXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("stuff")) {
        qWarning() << "Parsing Entry from invalid XML";
        return false;
    }

    d->mCategory = xmldata.attribute(QStringLiteral("category"));

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == QLatin1String("name")) {
            // TODO maybe do something with the language attribute? QString lang = e.attribute("lang");
            d->mName = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("author")) {
            QString email = e.attribute(QStringLiteral("email"));
            QString jabber = e.attribute(QStringLiteral("im"));
            QString homepage = e.attribute(QStringLiteral("homepage"));
            d->mAuthor.setName(e.text().trimmed());
            d->mAuthor.setEmail(email);
            d->mAuthor.setJabber(jabber);
            d->mAuthor.setHomepage(homepage);
        } else if (e.tagName() == QLatin1String("providerid")) {
            d->mProviderId = e.text();
        } else if (e.tagName() == QLatin1String("homepage")) {
            d->mHomepage = QUrl(e.text());
        } else if (e.tagName() == QLatin1String("licence")) { // krazy:exclude=spelling
            d->mLicense = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("summary")) {
            d->mSummary = e.text();
        } else if (e.tagName() == QLatin1String("changelog")) {
            d->mChangelog = e.text();
        } else if (e.tagName() == QLatin1String("version")) {
            d->mVersion = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("releasedate")) {
            d->mReleaseDate = QDate::fromString(e.text().trimmed(), Qt::ISODate);
        } else if (e.tagName() == QLatin1String("preview")) {
            // TODO support for all 6 image links
            d->mPreviewUrl[PreviewSmall1] = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("previewBig")) {
            d->mPreviewUrl[PreviewBig1] = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("payload")) {
            d->mPayload = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("rating")) {
            d->mRating = e.text().toInt();
        } else if (e.tagName() == QLatin1String("downloads")) {
            d->mDownloadCount = e.text().toInt();
        } else if (e.tagName() == QLatin1String("category")) {
            d->mCategory = e.text();
        } else if (e.tagName() == QLatin1String("signature")) {
            d->mSignature = e.text();
        } else if (e.tagName() == QLatin1String("checksum")) {
            d->mChecksum = e.text();
        } else if (e.tagName() == QLatin1String("installedfile")) {
            // TODO KF6 Add a "installeddirectory" entry to avoid
            // all the issues with the "/*" notation which is currently used as a workaround
            d->mInstalledFiles.append(e.text());
        } else if (e.tagName() == QLatin1String("id")) {
            d->mUniqueId = e.text();
        } else if (e.tagName() == QLatin1String("tags")) {
            d->mTags = e.text().split(QLatin1Char(','));
        } else if (e.tagName() == QLatin1String("status")) {
            QString statusText = e.text();
            if (statusText == QLatin1String("installed")) {
                qCDebug(KNEWSTUFFCORE) << "Found an installed entry in registry";
                d->mStatus = KNSCore::Entry::Installed;
            } else if (statusText == QLatin1String("updateable")) {
                d->mStatus = KNSCore::Entry::Updateable;
            }
        }
    }

    // Validation
    if (d->mName.isEmpty()) {
        qWarning() << "Entry: no name given";
        return false;
    }

    if (d->mUniqueId.isEmpty()) {
        if (!d->mPayload.isEmpty()) {
            d->mUniqueId = d->mPayload;
        } else {
            d->mUniqueId = d->mName;
        }
    }

    if (d->mPayload.isEmpty()) {
        qWarning() << "Entry: no payload URL given for: " << d->mName << " - " << d->mUniqueId;
        return false;
    }
    return true;
}

/**
 * get the xml string for the entry
 */
QDomElement KNSCore::Entry::entryXML() const
{
    Q_ASSERT(!d->mUniqueId.isEmpty());
    Q_ASSERT(!d->mProviderId.isEmpty());

    QDomDocument doc;

    QDomElement el = doc.createElement(QStringLiteral("stuff"));
    el.setAttribute(QStringLiteral("category"), d->mCategory);

    QString name = d->mName;

    QDomElement e;
    e = addElement(doc, el, QStringLiteral("name"), name);
    // todo: add language attribute
    (void)addElement(doc, el, QStringLiteral("providerid"), d->mProviderId);

    QDomElement author = addElement(doc, el, QStringLiteral("author"), d->mAuthor.name());
    if (!d->mAuthor.email().isEmpty()) {
        author.setAttribute(QStringLiteral("email"), d->mAuthor.email());
    }
    if (!d->mAuthor.homepage().isEmpty()) {
        author.setAttribute(QStringLiteral("homepage"), d->mAuthor.homepage());
    }
    if (!d->mAuthor.jabber().isEmpty()) {
        author.setAttribute(QStringLiteral("im"), d->mAuthor.jabber());
    }
    // FIXME: 'jabber' or 'im'? consult with kopete guys...
    addElement(doc, el, QStringLiteral("homepage"), d->mHomepage.url());
    (void)addElement(doc, el, QStringLiteral("licence"), d->mLicense); // krazy:exclude=spelling
    (void)addElement(doc, el, QStringLiteral("version"), d->mVersion);
    if ((d->mRating > 0) || (d->mDownloadCount > 0)) {
        (void)addElement(doc, el, QStringLiteral("rating"), QString::number(d->mRating));
        (void)addElement(doc, el, QStringLiteral("downloads"), QString::number(d->mDownloadCount));
    }
    if (!d->mSignature.isEmpty()) {
        (void)addElement(doc, el, QStringLiteral("signature"), d->mSignature);
    }
    if (!d->mChecksum.isEmpty()) {
        (void)addElement(doc, el, QStringLiteral("checksum"), d->mChecksum);
    }
    for (const QString &file : std::as_const(d->mInstalledFiles)) {
        (void)addElement(doc, el, QStringLiteral("installedfile"), file);
    }
    if (!d->mUniqueId.isEmpty()) {
        addElement(doc, el, QStringLiteral("id"), d->mUniqueId);
    }

    (void)addElement(doc, el, QStringLiteral("releasedate"), d->mReleaseDate.toString(Qt::ISODate));

    e = addElement(doc, el, QStringLiteral("summary"), d->mSummary);
    e = addElement(doc, el, QStringLiteral("changelog"), d->mChangelog);
    e = addElement(doc, el, QStringLiteral("preview"), d->mPreviewUrl[PreviewSmall1]);
    e = addElement(doc, el, QStringLiteral("previewBig"), d->mPreviewUrl[PreviewBig1]);
    e = addElement(doc, el, QStringLiteral("payload"), d->mPayload);
    e = addElement(doc, el, QStringLiteral("tags"), d->mTags.join(QLatin1Char(',')));

    if (d->mStatus == KNSCore::Entry::Installed) {
        (void)addElement(doc, el, QStringLiteral("status"), QStringLiteral("installed"));
    }
    if (d->mStatus == KNSCore::Entry::Updateable) {
        (void)addElement(doc, el, QStringLiteral("status"), QStringLiteral("updateable"));
    }

    return el;
}

void KNSCore::Entry::setEntryDeleted()
{
    setStatus(Entry::Deleted);
    d->mUnInstalledFiles = installedFiles();
    setInstalledFiles(QStringList());
}

void KNSCore::Entry::setEntryRequestedId(const QString &id)
{
    d->mRequestedUniqueId = id;
}

QString KNSCore::replaceBBCode(const QString &unformattedText)
{
    QString text(unformattedText);
    text.replace(QLatin1String("[b]"), QLatin1String("<b>"));
    text.replace(QLatin1String("[/b]"), QLatin1String("</b>"));
    text.replace(QLatin1String("[i]"), QLatin1String("<i>"));
    text.replace(QLatin1String("[/i]"), QLatin1String("</i>"));
    text.replace(QLatin1String("[u]"), QLatin1String("<i>"));
    text.replace(QLatin1String("[/u]"), QLatin1String("</i>"));
    text.replace(QLatin1String("\\\""), QLatin1String("\""));
    text.replace(QLatin1String("\\\'"), QLatin1String("\'"));
    text.replace(QLatin1String("[li]"), QLatin1String("* ")); // TODO: better replacement for list elements?
    text.remove(QStringLiteral("[/li]"));
    text.remove(QStringLiteral("[url]"));
    text.remove(QStringLiteral("[/url]"));
    return text;
}

QDebug KNSCore::operator<<(QDebug debug, const KNSCore::Entry &entry)
{
    QDebugStateSaver saver(debug);

    const static QMetaEnum metaEnum = QMetaEnum::fromType<KNSCore::Entry::Status>();
    bool deleted = entry.status() == Entry::Status::Deleted;

    debug.nospace() << "KNSCore::Entry(uniqueId: " << entry.uniqueId() << ", name:" << entry.name() << ", status: " << metaEnum.valueToKey(entry.status())
                    << ", " << (deleted ? "uninstalled" : "installed") << "Files: " // When the entry is installed, it can not have uninstalledFiles
                    << (deleted ? entry.uninstalledFiles() : entry.installedFiles()) << ')';
    return debug;
}

#include "moc_entry.cpp"
