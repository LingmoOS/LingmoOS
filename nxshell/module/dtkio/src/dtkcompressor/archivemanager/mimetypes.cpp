// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mimetypes.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QDebug>
#include <QProcess>
DCOMPRESSOR_BEGIN_NAMESPACE
CustomMimeType determineMimeType(const QString &filename)
{
    QMimeDatabase db;
    CustomMimeType stMimeType;

    QFileInfo fileinfo(filename);
    QString inputFile = filename;

    // #328815: since detection-by-content does not work for compressed tar archives (see below why)
    // we cannot rely on it when the archive extension is wrong; we need to validate by hand.
    if (fileinfo.completeSuffix().toLower().remove(QRegularExpression(QStringLiteral("[^a-z\\.]"))).contains(QStringLiteral("tar."))) {
        inputFile.chop(fileinfo.completeSuffix().length());
        QString cleanExtension(fileinfo.completeSuffix().toLower());

        // tar.bz2 and tar.lz4 need special treatment since they contain numbers.
        bool isBZ2 = false;
        bool isLZ4 = false;
        bool is7Z = false;
        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.bz2"))) {
            cleanExtension.remove(QStringLiteral("bz2"));
            isBZ2 = true;
        }

        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.lz4"))) {
            cleanExtension.remove(QStringLiteral("lz4"));
            isLZ4 = true;
        }

        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.7z"))) {
            cleanExtension.remove(QStringLiteral("7z"));
            is7Z = true;
        }

        // We remove non-alpha chars from the filename extension, but not periods.
        // If the filename is e.g. "foo.tar.gz.1", we get the "foo.tar.gz." string,
        // so we need to manually drop the last period character from it.
        cleanExtension.remove(QRegularExpression(QStringLiteral("[^a-z\\.]")));
        if (cleanExtension.endsWith(QLatin1Char('.'))) {
            cleanExtension.chop(1);
        }

        // Re-add extension for tar.bz2 and tar.lz4.
        if (isBZ2) {
            cleanExtension.append(QStringLiteral(".bz2"));
        }

        if (isLZ4) {
            cleanExtension.append(QStringLiteral(".lz4"));
        }

        if (is7Z) {
            cleanExtension.append(QStringLiteral(".7z"));
        }

        inputFile += cleanExtension;
    }

    QMimeType mimeFromExtension = db.mimeTypeForFile(inputFile, QMimeDatabase::MatchExtension);
    QMimeType mimeFromContent = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);

    // mimeFromContent will be "application/octet-stream" when file is
    // unreadable, so use extension.
    if (!fileinfo.isReadable()) {
        stMimeType.m_mimeType = mimeFromExtension;
        return stMimeType;
    }

    // Compressed tar-archives are detected as single compressed files when
    // detecting by content. The following code fixes detection of tar.gz, tar.bz2, tar.xz,
    // tar.lzo, tar.lz, tar.lrz and tar.zst.
    if ((mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-compressed-tar"))
         && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/gzip")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-bzip-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-bzip")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-xz-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-xz")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tarz"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-compress")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tzo"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzop")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lzip-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzip")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lrzip-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lrzip")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lz4-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lz4")))
        || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-zstd-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/zstd")))) {
        stMimeType.m_mimeType = mimeFromExtension;
        return stMimeType;
    }

    if (mimeFromContent.isDefault()) {

        QProcess process;
        QStringList args;
        args << "--mime-type" << filename;
        process.setProgram("file");
        process.setArguments(args);
        process.start();
        process.waitForFinished();
        const QString output = QString::fromUtf8(process.readAllStandardOutput());

        stMimeType.m_bUnKnown = true;
        if (output.contains("application/octet-stream")) {
            stMimeType.m_strTypeName = mimeFromExtension.name();
            return stMimeType;
        } else if (output.contains("application/x-chrome-extension")) {
            stMimeType.m_strTypeName = "application/x-chrome-extension";
            return stMimeType;
        } else if (output.contains("application/zip")) {
            stMimeType.m_strTypeName = "application/zip";
            return stMimeType;
        } else if (output.contains("application/x-iso9660-image")) {
            stMimeType.m_strTypeName = "application/x-iso9660-image";
            return stMimeType;
        } else {
            stMimeType.m_bUnKnown = false;
            stMimeType.m_mimeType = mimeFromExtension;
            return stMimeType;
        }
    }

    stMimeType.m_bUnKnown = false;
    if (mimeFromExtension != mimeFromContent) {
        if ((mimeFromContent.inherits(QStringLiteral("text/x-qml")) && fileinfo.completeSuffix().toLower().contains(QStringLiteral("rar")))
            || (mimeFromContent.name() == QStringLiteral("image/svg+xml") && mimeFromExtension.name() == QStringLiteral("application/zip"))) {
            stMimeType.m_mimeType = mimeFromExtension;
            return stMimeType;
        }
    }

    stMimeType.m_mimeType = mimeFromContent;
    return stMimeType;
}
DCOMPRESSOR_END_NAMESPACE
