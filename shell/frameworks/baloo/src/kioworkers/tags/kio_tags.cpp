/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2017-2018 James D. Smith <smithjd15@gmail.com>
    SPDX-FileCopyrightText: 2020 Stefan Brüns <bruns@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kio_tags.h"
#include "kio_tags_debug.h"

#include <QUrl>

#include <KLocalizedString>
#include <KUser>
#include <KIO/Job>

#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>

#include "file.h"
#include "taglistjob.h"
#include "../common/udstools.h"

#include "term.h"

using namespace Baloo;

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.tags" FILE "tags.json")
};

TagsProtocol::TagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : KIO::ForwardingWorkerBase("tags", pool_socket, app_socket)
{
}

TagsProtocol::~TagsProtocol()
{
}

KIO::WorkerResult TagsProtocol::listDir(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
        case FileUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "list() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_CANNOT_ENTER_DIRECTORY, result.decodedUrl);
        case TagUrl:
            listEntries(result.pathUDSResults);
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::stat(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "stat() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
        case FileUrl:
            return ForwardingWorkerBase::stat(result.fileUrl);
        case TagUrl:
            for (const KIO::UDSEntry& entry : std::as_const(result.pathUDSResults)) {
                if (entry.stringValue(KIO::UDSEntry::UDS_EXTRA) == result.tag) {
                    statEntry(entry);
                    break;
                }
            }
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::copy(const QUrl& src, const QUrl& dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    ParseResult srcResult = parseUrl(src);
    ParseResult dstResult = parseUrl(dest, QList<ParseFlags>() << ChopLastSection << LazyValidation);

    if (srcResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << srcResult.decodedUrl << "copy() invalid src url";
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, srcResult.decodedUrl);
    } else if (dstResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << dstResult.decodedUrl << "copy() invalid dest url";
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, dstResult.decodedUrl);
    }

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& newTag) {
        qCDebug(KIO_TAGS) << md.filePath() << "adding tag" << newTag;
        QStringList tags = md.tags();
        tags.append(newTag);
        md.setTags(tags);
    };

    if (srcResult.metaData.tags().contains(dstResult.tag)) {
        qCWarning(KIO_TAGS) << srcResult.fileUrl.toLocalFile() << "file already has tag" << dstResult.tag;
        infoMessage(i18n("File %1 already has tag %2", srcResult.fileUrl.toLocalFile(), dstResult.tag));
    } else if (dstResult.urlType == TagUrl) {
        rewriteTags(srcResult.metaData, dstResult.tag);
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::get(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "get() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
        case FileUrl:
            return ForwardingWorkerBase::get(result.fileUrl);
        case TagUrl:
            return KIO::WorkerResult::fail(KIO::ERR_UNSUPPORTED_ACTION, result.decodedUrl);
    }
    Q_UNREACHABLE();
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(flags);

    ParseResult srcResult = parseUrl(src);
    ParseResult dstResult;

    if (srcResult.urlType == FileUrl) {
        dstResult = parseUrl(dest, QList<ParseFlags>() << ChopLastSection);
    } else if (srcResult.urlType == TagUrl) {
        dstResult = parseUrl(dest, QList<ParseFlags>() << LazyValidation);
    }

    if (srcResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << srcResult.decodedUrl << "rename() invalid src url";
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, srcResult.decodedUrl);
    } else if (dstResult.urlType == InvalidUrl) {
        qCWarning(KIO_TAGS) << dstResult.decodedUrl << "rename() invalid dest url";
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, dstResult.decodedUrl);
    }

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& oldTag, const QString& newTag) {
        qCDebug(KIO_TAGS) << md.filePath() << "swapping tag" << oldTag << "with" << newTag;
        QStringList tags = md.tags();
        tags.removeAll(oldTag);
        tags.append(newTag);
        md.setTags(tags);
    };

    if (srcResult.metaData.tags().contains(dstResult.tag)) {
        qCWarning(KIO_TAGS) << srcResult.fileUrl.toLocalFile() << "file already has tag" << dstResult.tag;
        infoMessage(i18n("File %1 already has tag %2", srcResult.fileUrl.toLocalFile(), dstResult.tag));
    } else if (srcResult.urlType == FileUrl) {
        rewriteTags(srcResult.metaData, srcResult.tag, dstResult.tag);
    } else if (srcResult.urlType == TagUrl) {
        ResultIterator it = srcResult.query.exec();
        while (it.next()) {
            KFileMetaData::UserMetaData md(it.filePath());
            if (it.filePath() == srcResult.fileUrl.toLocalFile()) {
                rewriteTags(md, srcResult.tag, dstResult.tag);
            } else if (srcResult.fileUrl.isEmpty()) {
                const auto tags = md.tags();
                for (const QString& tag : tags) {
                    if (tag == srcResult.tag || (tag.startsWith(srcResult.tag + QLatin1Char('/')))) {
                        QString newTag = tag;
                        newTag.replace(srcResult.tag, dstResult.tag, Qt::CaseInsensitive);
                        rewriteTags(md, tag, newTag);
                    }
                }
            }
        }
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::del(const QUrl& url, bool isfile)
{
    Q_UNUSED(isfile);

    ParseResult result = parseUrl(url);

    auto rewriteTags = [] (KFileMetaData::UserMetaData& md, const QString& tag) {
        qCDebug(KIO_TAGS) << md.filePath() << "removing tag" << tag;
        QStringList tags = md.tags();
        tags.removeAll(tag);
        md.setTags(tags);
    };

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "del() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
        case FileUrl:
        case TagUrl:
            ResultIterator it = result.query.exec();
            while (it.next()) {
                KFileMetaData::UserMetaData md(it.filePath());
                if (it.filePath() == result.fileUrl.toLocalFile()) {
                    rewriteTags(md, result.tag);
                } else if (result.fileUrl.isEmpty()) {
                    const auto tags = md.tags();
                    for (const QString &tag : tags) {
                        if ((tag == result.tag) || (tag.startsWith(result.tag + QLatin1Char('/'), Qt::CaseInsensitive))) {
                            rewriteTags(md, tag);
                        }
                    }
                }
            }
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::mimetype(const QUrl& url)
{
    ParseResult result = parseUrl(url);

    switch(result.urlType) {
        case InvalidUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "mimetype() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
        case FileUrl:
            return ForwardingWorkerBase::mimetype(result.fileUrl);
        case TagUrl:
            mimeType(QStringLiteral("inode/directory"));
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TagsProtocol::mkdir(const QUrl& url, int permissions)
{
    Q_UNUSED(permissions);

    ParseResult result = parseUrl(url, QList<ParseFlags>() << LazyValidation);

    switch(result.urlType) {
        case InvalidUrl:
        case FileUrl:
            qCWarning(KIO_TAGS) << result.decodedUrl << "mkdir() invalid url";
            return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, result.decodedUrl);
        case TagUrl:
            m_unassignedTags << result.tag;
    }

    return KIO::WorkerResult::pass();
}

bool TagsProtocol::rewriteUrl(const QUrl& url, QUrl& newURL)
{
    Q_UNUSED(url);
    Q_UNUSED(newURL);

    return false;
}

TagsProtocol::ParseResult TagsProtocol::parseUrl(const QUrl& url, const QList<ParseFlags> &flags)
{
    TagsProtocol::ParseResult result;
    result.decodedUrl = QUrl::fromPercentEncoding(url.toString().toUtf8());

    if ((url.scheme() == QLatin1String("tags")) && result.decodedUrl.length()>6 && result.decodedUrl.at(6) == QLatin1Char('/')) {
        result.urlType = InvalidUrl;
        return result;
    }

    auto createUDSEntryForTag = [] (const QString& tagSection, const QString& tag) {
        KIO::UDSEntry uds;
        uds.reserve(9);
        uds.fastInsert(KIO::UDSEntry::UDS_NAME, tagSection);
        uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
        uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
        uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        uds.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("tag"));
        uds.fastInsert(KIO::UDSEntry::UDS_EXTRA, tag);

        QString displayType;
        QString displayName;

        // a tag/folder
        if (tagSection == tag) {
            displayType = i18nc("This is a noun", "Tag");
            displayName = tag.section(QLatin1Char('/'), -1);
        }

        // a tagged file
        else if (!tag.isEmpty()) {
            displayType = i18nc("This is a noun", "Tag Fragment");
            if (tagSection == QStringLiteral("..")) {
                displayName = tag.section(QLatin1Char('/'), -2);
            } else if (tagSection == QStringLiteral(".")) {
                displayName = tag.section(QLatin1Char('/'), -1);
            } else {
                displayName = tagSection;
            }
        }

        // The root folder
        else {
            displayType = i18n("All Tags");
            displayName = i18n("All Tags");
        }

        uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_TYPE, displayType);
        uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayName);

        return uds;
    };

    TagListJob* tagJob = new TagListJob();
    if (!tagJob->exec()) {
        qCWarning(KIO_TAGS) << "tag fetch failed:" << tagJob->errorString();
        return result;
    }

    if (url.isLocalFile()) {
        result.urlType = FileUrl;
        result.fileUrl = url;
        result.metaData = KFileMetaData::UserMetaData(url.toLocalFile());
    } else if (url.scheme() == QLatin1String("tags")) {
        bool validTag = flags.contains(LazyValidation);

        // Determine the tag from the URL.
        result.tag = result.decodedUrl;
        result.tag.remove(url.scheme() + QLatin1Char(':'));
        result.tag = QDir::cleanPath(result.tag);
        while (result.tag.startsWith(QLatin1Char('/'))) {
            result.tag.remove(0, 1);
        }

        // Extract any local file path from the URL.
        QString tag = result.tag.section(QDir::separator(), 0, -2);
        QString fileName = result.tag.section(QDir::separator(), -1, -1);
        int pos = 0;

        // Extract and remove any multiple filename suffix from the file name.
        QRegularExpression regexp(QStringLiteral("\\s\\((\\d+)\\)$"));
        QRegularExpressionMatch regMatch = regexp.match(fileName);
        if (regMatch.hasMatch()) {
            pos = regMatch.captured(1).toInt();

            fileName.remove(regexp);
        }

        Query q;
        q.setSearchString(QStringLiteral("tag=\"%1\" AND filename=\"%2\"").arg(tag, fileName));
        ResultIterator it = q.exec();

        int i = 0;
        while (it.next()) {
            result.fileUrl = QUrl::fromLocalFile(it.filePath());
            result.metaData = KFileMetaData::UserMetaData(it.filePath());

            if (i == pos) {
                break;
            } else {
                i++;
            }
        }

        if (!result.fileUrl.isEmpty() || flags.contains(ChopLastSection)) {
            result.tag = result.tag.section(QDir::separator(), 0, -2);
        }

        validTag = validTag || result.tag.isEmpty();

        if (!result.tag.isEmpty()) {
            // Create a query to find files that may be in the operation's scope.
            QString query = result.tag;
            query.prepend(QStringLiteral("tag:"));
            query.replace(QLatin1Char(' '), QStringLiteral(" AND tag:"));
            query.replace(QLatin1Char('/'), QStringLiteral(" AND tag:"));
            result.query.setSearchString(query);

            qCDebug(KIO_TAGS) << result.decodedUrl << "url query:" << query;
        }

        // Create the tag directory entries.
        int index = result.tag.count(QLatin1Char('/')) + (result.tag.isEmpty() ? 0 : 1);
        QStringList tagPaths;

        const QStringList tags = QStringList() << tagJob->tags() << m_unassignedTags;
        for (const QString& tag : tags) {
            if (result.tag.isEmpty() || (tag.startsWith(result.tag, Qt::CaseInsensitive))) {
                QString tagSection = tag.section(QLatin1Char('/'), index, index, QString::SectionSkipEmpty);
                if (!tagPaths.contains(tagSection, Qt::CaseInsensitive) && !tagSection.isEmpty()) {
                    result.pathUDSResults << createUDSEntryForTag(tagSection, tag);
                    tagPaths << tagSection;
                }
            }

            validTag = validTag || tag.startsWith(result.tag, Qt::CaseInsensitive);
        }

        if (validTag && result.fileUrl.isEmpty()) {
            result.urlType = TagUrl;
        } else if (validTag && !result.fileUrl.isEmpty()) {
            result.urlType = FileUrl;
        }
    }

    if (result.urlType == FileUrl) {
        return result;
    } else {
        result.pathUDSResults << createUDSEntryForTag(QStringLiteral("."), result.tag);
    }

    // The root tag url has no file entries.
    if (result.tag.isEmpty()) {
        return result;
    } else {
        result.pathUDSResults << createUDSEntryForTag(QStringLiteral(".."), result.tag);
    }

    // Query for any files associated with the tag.
    Query q;
    q.setSearchString(QStringLiteral("tag=\"%1\"").arg(result.tag));
    ResultIterator it = q.exec();
    QList<QString> resultNames;
    UdsFactory udsf;

    while (it.next()) {
        KIO::UDSEntry uds = udsf.createUdsEntry(it.filePath());
        if (uds.count() == 0) {
	    continue;
	}

	const QUrl url(uds.stringValue(KIO::UDSEntry::UDS_URL));
	auto dupCount = resultNames.count(url.fileName());
        if (dupCount > 0) {
            uds.replace(KIO::UDSEntry::UDS_NAME, url.fileName() + QStringLiteral(" (%1)").arg(dupCount));
        }

        qCDebug(KIO_TAGS) << result.tag << "adding file:" << uds.stringValue(KIO::UDSEntry::UDS_NAME);

        resultNames << url.fileName();
        result.pathUDSResults << uds;
    }

    return result;
}

extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_tags"));
        Baloo::TagsProtocol worker(argv[2], argv[3]);
        worker.dispatchLoop();
        return 0;
    }
}

#include "kio_tags.moc"
#include "moc_kio_tags.cpp"
