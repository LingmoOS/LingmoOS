/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2005-2012 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kurlmimedata.h"
#include "config-kdirwatch.h"

#if HAVE_QTDBUS // not used outside dbus/xdg-portal related code
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <optional>

#include <QMimeData>
#include <QStringList>

#include "kcoreaddons_debug.h"
#if HAVE_QTDBUS
#include "org.freedesktop.portal.FileTransfer.h"
#include "org.kde.KIOFuse.VFS.h"
#endif

#include "kurlmimedata_p.h"

static QString kdeUriListMime()
{
    return QStringLiteral("application/x-kde4-urilist");
} // keep this name "kde4" for compat.

static QByteArray uriListData(const QList<QUrl> &urls)
{
    // compatible with qmimedata.cpp encoding of QUrls
    QByteArray result;
    for (int i = 0; i < urls.size(); ++i) {
        result += urls.at(i).toEncoded();
        result += "\r\n";
    }
    return result;
}

void KUrlMimeData::setUrls(const QList<QUrl> &urls, const QList<QUrl> &mostLocalUrls, QMimeData *mimeData)
{
    // Export the most local urls as text/uri-list and plain text, for non KDE apps.
    mimeData->setUrls(mostLocalUrls); // set text/uri-list and text/plain

    // Export the real KIO urls as a kde-specific mimetype
    mimeData->setData(kdeUriListMime(), uriListData(urls));
}

void KUrlMimeData::setMetaData(const MetaDataMap &metaData, QMimeData *mimeData)
{
    QByteArray metaDataData; // :)
    for (auto it = metaData.cbegin(); it != metaData.cend(); ++it) {
        metaDataData += it.key().toUtf8();
        metaDataData += "$@@$";
        metaDataData += it.value().toUtf8();
        metaDataData += "$@@$";
    }
    mimeData->setData(QStringLiteral("application/x-kio-metadata"), metaDataData);
}

QStringList KUrlMimeData::mimeDataTypes()
{
    return QStringList{kdeUriListMime(), QStringLiteral("text/uri-list")};
}

static QList<QUrl> extractKdeUriList(const QMimeData *mimeData)
{
    QList<QUrl> uris;
    const QByteArray ba = mimeData->data(kdeUriListMime());
    // Code from qmimedata.cpp
    QList<QByteArray> urls = ba.split('\n');
    uris.reserve(urls.size());
    for (int i = 0; i < urls.size(); ++i) {
        QByteArray data = urls.at(i).trimmed();
        if (!data.isEmpty()) {
            uris.append(QUrl::fromEncoded(data));
        }
    }
    return uris;
}

#if HAVE_QTDBUS
static QString kioFuseServiceName()
{
    return QStringLiteral("org.kde.KIOFuse");
}

static QString portalServiceName()
{
    return QStringLiteral("org.freedesktop.portal.Documents");
}

static bool isKIOFuseAvailable()
{
    static bool available = QDBusConnection::sessionBus().interface()
        && QDBusConnection::sessionBus().interface()->activatableServiceNames().value().contains(kioFuseServiceName());
    return available;
}

bool KUrlMimeData::isDocumentsPortalAvailable()
{
    static bool available =
        QDBusConnection::sessionBus().interface() && QDBusConnection::sessionBus().interface()->activatableServiceNames().value().contains(portalServiceName());
    return available;
}

static QString portalFormat()
{
    return QStringLiteral("application/vnd.portal.filetransfer");
}

static QList<QUrl> extractPortalUriList(const QMimeData *mimeData)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());
    static std::pair<QByteArray, QList<QUrl>> cache;
    const auto transferId = mimeData->data(portalFormat());
    qCDebug(KCOREADDONS_DEBUG) << "Picking up portal urls from transfer" << transferId;
    if (std::get<QByteArray>(cache) == transferId) {
        const auto uris = std::get<QList<QUrl>>(cache);
        qCDebug(KCOREADDONS_DEBUG) << "Urls from portal cache" << uris;
        return uris;
    }
    auto iface =
        new OrgFreedesktopPortalFileTransferInterface(portalServiceName(), QStringLiteral("/org/freedesktop/portal/documents"), QDBusConnection::sessionBus());
    const QDBusReply<QStringList> reply = iface->RetrieveFiles(QString::fromUtf8(transferId), {});
    if (!reply.isValid()) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to retrieve files from portal:" << reply.error();
        return {};
    }
    const QStringList list = reply.value();
    QList<QUrl> uris;
    uris.reserve(list.size());
    for (const auto &path : list) {
        uris.append(QUrl::fromLocalFile(path));
    }
    qCDebug(KCOREADDONS_DEBUG) << "Urls from portal" << uris;
    cache = std::make_pair(transferId, uris);
    return uris;
}

static QString sourceIdMime()
{
    return QStringLiteral("application/x-kde-source-id");
}

static QString sourceId()
{
    return QDBusConnection::sessionBus().baseService();
}

void KUrlMimeData::setSourceId(QMimeData *mimeData)
{
    mimeData->setData(sourceIdMime(), sourceId().toUtf8());
}

static bool hasSameSourceId(const QMimeData *mimeData)
{
    return mimeData->hasFormat(sourceIdMime()) && mimeData->data(sourceIdMime()) == sourceId().toUtf8();
}

#endif

QList<QUrl> KUrlMimeData::urlsFromMimeData(const QMimeData *mimeData, DecodeOptions decodeOptions, MetaDataMap *metaData)
{
    QList<QUrl> uris;

#if HAVE_QTDBUS
    if (!hasSameSourceId(mimeData) && isDocumentsPortalAvailable() && mimeData->hasFormat(portalFormat())) {
        uris = extractPortalUriList(mimeData);
        if (static const auto force = qEnvironmentVariableIntValue("KCOREADDONS_FORCE_DOCUMENTS_PORTAL"); force == 1) {
            // The environment variable is FOR TESTING ONLY!
            // It is used to prevent the fallback logic from running.
            return uris;
        }
    }
#endif

    if (uris.isEmpty()) {
        if (decodeOptions.testFlag(PreferLocalUrls)) {
            // Extracting uris from text/uri-list, use the much faster QMimeData method urls()
            uris = mimeData->urls();
            if (uris.isEmpty()) {
                uris = extractKdeUriList(mimeData);
            }
        } else {
            uris = extractKdeUriList(mimeData);
            if (uris.isEmpty()) {
                uris = mimeData->urls();
            }
        }
    }

    if (metaData) {
        const QByteArray metaDataPayload = mimeData->data(QStringLiteral("application/x-kio-metadata"));
        if (!metaDataPayload.isEmpty()) {
            QString str = QString::fromUtf8(metaDataPayload.constData());
            Q_ASSERT(str.endsWith(QLatin1String("$@@$")));
            str.chop(4);
            const QStringList lst = str.split(QStringLiteral("$@@$"));
            bool readingKey = true; // true, then false, then true, etc.
            QString key;
            for (const QString &s : lst) {
                if (readingKey) {
                    key = s;
                } else {
                    metaData->insert(key, s);
                }
                readingKey = !readingKey;
            }
            Q_ASSERT(readingKey); // an odd number of items would be, well, odd ;-)
        }
    }
    return uris;
}

#if HAVE_QTDBUS
static QStringList urlListToStringList(const QList<QUrl> urls)
{
    QStringList list;
    for (const auto &url : urls) {
        list << url.toLocalFile();
    }
    return list;
}

static std::optional<QStringList> fuseRedirect(QList<QUrl> urls, bool onlyLocalFiles)
{
    qCDebug(KCOREADDONS_DEBUG) << "mounting urls with fuse" << urls;

    // Fuse redirection only applies if the list contains non-local files.
    if (onlyLocalFiles) {
        return urlListToStringList(urls);
    }

    OrgKdeKIOFuseVFSInterface kiofuse_iface(kioFuseServiceName(), QStringLiteral("/org/kde/KIOFuse"), QDBusConnection::sessionBus());
    struct MountRequest {
        QDBusPendingReply<QString> reply;
        int urlIndex;
        QString basename;
    };
    QList<MountRequest> requests;
    requests.reserve(urls.count());
    for (int i = 0; i < urls.count(); ++i) {
        QUrl url = urls.at(i);
        if (!url.isLocalFile()) {
            const QString path(url.path());
            const int slashes = path.count(QLatin1Char('/'));
            QString basename;
            if (slashes > 1) {
                url.setPath(path.section(QLatin1Char('/'), 0, slashes - 1));
                basename = path.section(QLatin1Char('/'), slashes, slashes);
            }
            requests.push_back({kiofuse_iface.mountUrl(url.toString()), i, basename});
        }
    }

    for (auto &request : requests) {
        request.reply.waitForFinished();
        if (request.reply.isError()) {
            qWarning() << "FUSE request failed:" << request.reply.error();
            return std::nullopt;
        }

        urls[request.urlIndex] = QUrl::fromLocalFile(request.reply.value() + QLatin1Char('/') + request.basename);
    };

    qCDebug(KCOREADDONS_DEBUG) << "mounted urls with fuse, maybe" << urls;

    return urlListToStringList(urls);
}
#endif

bool KUrlMimeData::exportUrlsToPortal(QMimeData *mimeData)
{
#if HAVE_QTDBUS
    if (!isDocumentsPortalAvailable()) {
        return false;
    }
    QList<QUrl> urls = mimeData->urls();

    bool onlyLocalFiles = true;
    for (const auto &url : urls) {
        const auto isLocal = url.isLocalFile();
        if (!isLocal) {
            onlyLocalFiles = false;

            // For the time being the fuse redirection is opt-in because we later need to open() the files
            // and this is an insanely expensive operation involving a stat() for remote URLs that we can't
            // really get rid of. We'll need a way to avoid the open().
            // https://bugs.kde.org/show_bug.cgi?id=457529
            // https://github.com/flatpak/xdg-desktop-portal/issues/961
            static const auto fuseRedirect = qEnvironmentVariableIntValue("KCOREADDONS_FUSE_REDIRECT");
            if (!fuseRedirect) {
                return false;
            }

            // some remotes, fusing is enabled, but kio-fuse is unavailable -> cannot run this url list through the portal
            if (!isKIOFuseAvailable()) {
                qWarning() << "kio-fuse is missing";
                return false;
            }
        } else {
            const QFileInfo info(url.toLocalFile());
            if (info.isDir()) {
                // XDG Document Portal doesn't support directories and silently drops them.
                return false;
            }
            if (info.isSymbolicLink()) {
                // XDG Document Portal also doesn't support symlinks since it doesn't let us open the fd O_NOFOLLOW.
                // https://github.com/flatpak/xdg-desktop-portal/issues/961#issuecomment-1573646299
                return false;
            }
        }
    }

    auto iface =
        new OrgFreedesktopPortalFileTransferInterface(portalServiceName(), QStringLiteral("/org/freedesktop/portal/documents"), QDBusConnection::sessionBus());

    // Do not autostop, we'll stop once our mimedata disappears (i.e. the drag operation has finished);
    // Otherwise not-wellbehaved clients that read the urls multiple times will trip the automatic-transfer-
    // closing-upon-read inside the portal and have any reads, but the first, not properly resolve anymore.
    const QString transferId = iface->StartTransfer({{QStringLiteral("autostop"), QVariant::fromValue(false)}});
    mimeData->setData(QStringLiteral("application/vnd.portal.filetransfer"), QFile::encodeName(transferId));
    setSourceId(mimeData);

    auto optionalPaths = fuseRedirect(urls, onlyLocalFiles);
    if (!optionalPaths.has_value()) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to mount with fuse!";
        return false;
    }

    // Prevent running into "too many open files" errors.
    // Because submission of calls happens on the qdbus thread we may be feeding
    // it QDBusUnixFileDescriptors faster than it can submit them over the wire, this would eventually
    // lead to running into the open file cap since the QDBusUnixFileDescriptor hold
    // an open FD until their call has been made.
    // To prevent this from happening we collect a submission batch, make the call and **wait** for
    // the call to succeed.
    FDList pendingFds;
    static constexpr decltype(pendingFds.size()) maximumBatchSize = 16;
    pendingFds.reserve(maximumBatchSize);

    const auto addFilesAndClear = [transferId, &iface, &pendingFds]() {
        if (pendingFds.isEmpty()) {
            return;
        }
        auto reply = iface->AddFiles(transferId, pendingFds, {});
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(KCOREADDONS_DEBUG) << "Some files could not be exported. " << reply.error();
        }
        pendingFds.clear();
    };

    for (const auto &path : optionalPaths.value()) {
        const int fd = open(QFile::encodeName(path).constData(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
        if (fd == -1) {
            const int error = errno;
            qCWarning(KCOREADDONS_DEBUG) << "Failed to open" << path << strerror(error);
        }
        pendingFds << QDBusUnixFileDescriptor(fd);
        close(fd);

        if (pendingFds.size() >= maximumBatchSize) {
            addFilesAndClear();
        }
    }
    addFilesAndClear();

    QObject::connect(mimeData, &QObject::destroyed, iface, [transferId, iface] {
        iface->StopTransfer(transferId);
        iface->deleteLater();
    });
    QObject::connect(iface, &OrgFreedesktopPortalFileTransferInterface::TransferClosed, mimeData, [iface]() {
        iface->deleteLater();
    });

    return true;
#else
    Q_UNUSED(mimeData);
    return false;
#endif
}
