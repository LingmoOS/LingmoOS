/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "basicindexingjob.h"
#include "termgenerator.h"
#include "idutils.h"

#include <QStringList>
#include <QFile>

#include <KFileMetaData/Types>
#include <KFileMetaData/UserMetaData>

using namespace Baloo;

BasicIndexingJob::BasicIndexingJob(const QString& filePath, const QString& mimetype,
                                   IndexingLevel level)
    : m_filePath(filePath)
    , m_mimetype(mimetype)
    , m_indexingLevel(level)
{
    if (m_filePath.endsWith(QLatin1Char('/'))) {
	m_filePath.chop(1);
    }
}

namespace {

void indexXAttr(const QString& url, Document& doc)
{
    KFileMetaData::UserMetaData userMetaData(url);

    using Attribute = KFileMetaData::UserMetaData::Attribute;
    auto attributes = userMetaData.queryAttributes(Attribute::Tags |
        Attribute::Rating | Attribute::Comment);
    if (attributes == Attribute::None) {
	return;
    }

    TermGenerator tg(doc);

    const QStringList tags = userMetaData.tags();
    for (const QString& tag : tags) {
        tg.indexXattrText(tag, QByteArray("TA"));
        doc.addXattrTerm(QByteArray("TAG-") + tag.toUtf8());
    }

    int rating = userMetaData.rating();
    if (rating) {
        doc.addXattrTerm(QByteArray("R") + QByteArray::number(rating));
    }

    QString comment = userMetaData.userComment();
    if (!comment.isEmpty()) {
        tg.indexXattrText(comment, QByteArray("C"));
    }
}

QVector<KFileMetaData::Type::Type> typesForMimeType(const QString& mimeType)
{
    using namespace KFileMetaData;
    QVector<Type::Type> types;
    types.reserve(2);

    // Basic types
    if (mimeType.startsWith(QLatin1String("audio/"))) {
        types << Type::Audio;
    }
    if (mimeType.startsWith(QLatin1String("video/"))) {
        types << Type::Video;
    }
    if (mimeType.startsWith(QLatin1String("image/"))) {
        types << Type::Image;
    }
    if (mimeType.startsWith(QLatin1String("text/"))) {
        types << Type::Text;
    }
    if (mimeType.contains(QLatin1String("document"))) {
        types << Type::Document;
    }

    if (mimeType.contains(QLatin1String("powerpoint"))) {
        types << Type::Presentation;
        types << Type::Document;
    }
    if (mimeType.contains(QLatin1String("excel"))) {
        types << Type::Spreadsheet;
        types << Type::Document;
    }
    // Compressed tar archives: "application/x-<compression>-compressed-tar"
    if ((mimeType.startsWith(QLatin1String("application/x-"))) &&
        (mimeType.endsWith(QLatin1String("-compressed-tar")))) {
        types << Type::Archive;
    }

    static QMultiHash<QString, Type::Type> typeMapper {
        {QStringLiteral("text/plain"), Type::Document},
        // MS Office
        {QStringLiteral("application/msword"), Type::Document},
        {QStringLiteral("application/x-scribus"), Type::Document},
        // The old pre-XML MS Office formats are already covered by the excel/powerpoint "contains" above:
        // - application/vnd.ms-powerpoint
        // - application/vnd.ms-excel
        // "openxmlformats-officedocument" and "opendocument" contain "document", i.e. already have Type::Document
        // - application/vnd.openxmlformats-officedocument.wordprocessingml.document
        // - application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
        // - application/vnd.openxmlformats-officedocument.presentationml.presentation
        // - application/vnd.oasis.opendocument.text
        // - application/vnd.oasis.opendocument.spreadsheet
        // - application/vnd.oasis.opendocument.presentation
        // Office 2007
        {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"), Type::Presentation},
        {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideshow"), Type::Presentation},
        {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.template"), Type::Presentation},
        {QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), Type::Spreadsheet},
        // Open Document Formats - https://en.wikipedia.org/wiki/OpenDocument_technical_specification
        {QStringLiteral("application/vnd.oasis.opendocument.presentation"), Type::Presentation},
        {QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), Type::Spreadsheet},
        {QStringLiteral("application/pdf"), Type::Document},
        {QStringLiteral("application/postscript"), Type::Document},
        {QStringLiteral("application/x-dvi"), Type::Document},
        {QStringLiteral("application/rtf"), Type::Document},
        // EBooks
        {QStringLiteral("application/epub+zip"), Type::Document},
        {QStringLiteral("application/vnd.amazon.mobi8-ebook"), Type::Document},
        {QStringLiteral("application/x-mobipocket-ebook"), Type::Document},
        // Graphic EBooks
        {QStringLiteral("application/vnd.comicbook-rar"), Type::Document},
        {QStringLiteral("application/vnd.comicbook+zip"), Type::Document},
        {QStringLiteral("application/x-cb7"), Type::Document},
        {QStringLiteral("application/x-cbt"), Type::Document},
        // Archives - https://en.wikipedia.org/wiki/List_of_archive_formats
        {QStringLiteral("application/gzip"), Type::Archive},
        {QStringLiteral("application/x-tar"), Type::Archive},
        {QStringLiteral("application/x-tarz"), Type::Archive},
        {QStringLiteral("application/x-arc"), Type::Archive},
        {QStringLiteral("application/x-archive"), Type::Archive},
        {QStringLiteral("application/x-bzip"), Type::Archive},
        {QStringLiteral("application/x-cpio"), Type::Archive},
        {QStringLiteral("application/x-lha"), Type::Archive},
        {QStringLiteral("application/x-lhz"), Type::Archive},
        {QStringLiteral("application/x-lrzip"), Type::Archive},
        {QStringLiteral("application/x-lz4"), Type::Archive},
        {QStringLiteral("application/x-lzip"), Type::Archive},
        {QStringLiteral("application/x-lzma"), Type::Archive},
        {QStringLiteral("application/x-lzop"), Type::Archive},
        {QStringLiteral("application/x-7z-compressed"), Type::Archive},
        {QStringLiteral("application/x-ace"), Type::Archive},
        {QStringLiteral("application/x-astrotite-afa"), Type::Archive},
        {QStringLiteral("application/x-alz"), Type::Archive},
        {QStringLiteral("application/vnd.android.package-archive"), Type::Archive},
        {QStringLiteral("application/x-arj"), Type::Archive},
        {QStringLiteral("application/vnd.ms-cab-compressed"), Type::Archive},
        {QStringLiteral("application/x-cfs-compressed"), Type::Archive},
        {QStringLiteral("application/x-dar"), Type::Archive},
        {QStringLiteral("application/x-lzh"), Type::Archive},
        {QStringLiteral("application/x-lzx"), Type::Archive},
        {QStringLiteral("application/vnd.rar"), Type::Archive},
        {QStringLiteral("application/x-stuffit"), Type::Archive},
        {QStringLiteral("application/x-stuffitx"), Type::Archive},
        {QStringLiteral("application/x-tzo"), Type::Archive},
        {QStringLiteral("application/x-ustar"), Type::Archive},
        {QStringLiteral("application/x-xar"), Type::Archive},
        {QStringLiteral("application/x-xz"), Type::Archive},
        {QStringLiteral("application/x-zoo"), Type::Archive},
        {QStringLiteral("application/zip"), Type::Archive},
        {QStringLiteral("application/zlib"), Type::Archive},
        {QStringLiteral("application/zstd"), Type::Archive},
        // WPS office
        {QStringLiteral("application/wps-office.doc"), Type::Document},
        {QStringLiteral("application/wps-office.xls"), Type::Document},
        {QStringLiteral("application/wps-office.xls"), Type::Spreadsheet},
        {QStringLiteral("application/wps-office.pot"), Type::Document},
        {QStringLiteral("application/wps-office.pot"), Type::Presentation},
        {QStringLiteral("application/wps-office.wps"), Type::Document},
        {QStringLiteral("application/wps-office.docx"), Type::Document},
        {QStringLiteral("application/wps-office.xlsx"), Type::Document},
        {QStringLiteral("application/wps-office.xlsx"), Type::Spreadsheet},
        {QStringLiteral("application/wps-office.pptx"), Type::Document},
        {QStringLiteral("application/wps-office.pptx"), Type::Presentation},
        // Other
        {QStringLiteral("text/markdown"), Type::Document},
        {QStringLiteral("image/vnd.djvu+multipage"), Type::Document},
        {QStringLiteral("application/x-lyx"), Type::Document}
    };

    auto hashIt = typeMapper.find(mimeType);
    while (hashIt != typeMapper.end() && hashIt.key() == mimeType) {
        types.append(hashIt.value());
        ++hashIt;
    }

    return types;
}
} // namespace

BasicIndexingJob::~BasicIndexingJob()
{
}

bool BasicIndexingJob::index()
{
    const QByteArray url = QFile::encodeName(m_filePath);
    auto lastSlash = url.lastIndexOf('/');

    const QByteArray fileName = url.mid(lastSlash + 1);
    const QByteArray filePath = url.left(lastSlash);

    QT_STATBUF statBuf;
    if (filePathToStat(filePath, statBuf) != 0) {
        return false;
    }

    Document doc;
    doc.setParentId(statBufToId(statBuf));

    if (filePathToStat(url, statBuf) != 0) {
        return false;
    }
    doc.setId(statBufToId(statBuf));
    doc.setUrl(url);

    TermGenerator tg(doc);
    tg.indexFileNameText(QFile::decodeName(fileName));
    if (statBuf.st_size == 0) {
        tg.indexText(QStringLiteral("application/x-zerosize"), QByteArray("M"));
    } else {
        tg.indexText(m_mimetype, QByteArray("M"));
    }

    // (Content) Modification time, Metadata (e.g. XAttr) change time
    doc.setMTime(statBuf.st_mtime);
    doc.setCTime(statBuf.st_ctime);

    if (S_ISDIR(statBuf.st_mode)) {
        static const QByteArray type = QByteArray("T") + QByteArray::number(static_cast<int>(KFileMetaData::Type::Folder));
        doc.addTerm(type);
        // For folders we do not need to go through file indexing, so we do not set contentIndexing

    } else if (statBuf.st_size > 0) {
        if (m_indexingLevel == MarkForContentIndexing) {
            doc.setContentIndexing(true);
        }
        // Types
        const QVector<KFileMetaData::Type::Type> tList = typesForMimeType(m_mimetype);
        for (KFileMetaData::Type::Type type : tList) {
            QByteArray num = QByteArray::number(static_cast<int>(type));
            doc.addTerm(QByteArray("T") + num);
        }
    }

    indexXAttr(m_filePath, doc);

    m_doc = doc;
    return true;
}
