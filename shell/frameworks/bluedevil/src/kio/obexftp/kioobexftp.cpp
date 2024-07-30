/*
 *  SPDX-FileCopyrightText: 2010-2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kioobexftp.h"
#include "bluedevil_kio_obexftp.h"
#include "transferfilejob.h"
#include "version.h"
#include <kio_version.h>

#include <unistd.h>

#include <QCoreApplication>
#include <QMimeData>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <KLocalizedString>

#include <BluezQt/ObexTransfer>
#include <BluezQt/PendingCall>

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.obexftp" FILE "obexftp.json")
};

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_obexftp protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioFtp slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

static QString urlDirectory(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.adjusted(QUrl::RemoveFilename).path();
}

static QString urlFileName(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.fileName();
}

static QUrl urlUpDir(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.adjusted(QUrl::RemoveFilename);
}

static bool urlIsRoot(const QUrl &url)
{
    const QString &directory = urlDirectory(url);
    return (directory.isEmpty() || directory == QLatin1String("/")) && urlFileName(url).isEmpty();
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : WorkerBase(QByteArrayLiteral("obexftp"), pool, app)
    , m_transfer(nullptr)
{
    m_kded = new org::kde::BlueDevil::ObexFtp(QStringLiteral("org.kde.kded6"), QStringLiteral("/modules/bluedevil"), QDBusConnection::sessionBus(), this);
}

void KioFtp::connectToHost()
{
    const QString &target = m_kded->preferredTarget(m_host);

    if (target != QLatin1String("ftp")) {
        if (createSession(target)) {
            return;
        }
        // Fallback to ftp
    }

    createSession(QStringLiteral("ftp"));
}

KIO::WorkerResult KioFtp::testConnection()
{
    if (!m_kded->isOnline().value()) {
        return KIO::WorkerResult::fail(KIO::ERR_WORKER_DEFINED, i18n("Obexd service is not running."));
    }

    connectToHost();

    if (!m_transfer) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_CONNECT, m_host);
    }
    return KIO::WorkerResult::pass();
}

bool KioFtp::createSession(const QString &target)
{
    QDBusPendingReply<QString> reply = m_kded->session(m_host, target);
    reply.waitForFinished();

    const QString &sessionPath = reply.value();

    if (reply.isError() || sessionPath.isEmpty()) {
        qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Create session error" << reply.error().name() << reply.error().message();
        delete m_transfer;
        m_transfer = nullptr;
        m_sessionPath.clear();
        return false;
    }

    if (m_sessionPath != sessionPath) {
        m_statMap.clear();
        delete m_transfer;
        m_transfer = new BluezQt::ObexFileTransfer(QDBusObjectPath(sessionPath));
        m_sessionPath = sessionPath;
    }

    return true;
}

KIO::WorkerResult KioFtp::listDir(const QUrl &url)
{
    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device…"));

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Asking for listFolder" << url.path();

    if (auto result = changeFolder(url.path()); !result.success()) {
        return result;
    }

    const auto [result, entries] = listFolder(url);
    if (!result.success()) {
        return result;
    }

    for (const KIO::UDSEntry &entry : entries) {
        listEntry(entry);
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "copy: " << src.url() << " to " << dest.url();

    return copyHelper(src, dest);
}

KIO::WorkerResult KioFtp::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    return KIO::WorkerResult::fail(KIO::ERR_UNSUPPORTED_ACTION, QString());
}

KIO::WorkerResult KioFtp::get(const QUrl &url)
{
    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "get" << url;

    QTemporaryFile tempFile(QStringLiteral("%1/kioftp_XXXXXX.%2").arg(QDir::tempPath(), urlFileName(url)));
    tempFile.open();

    if (auto result = copyHelper(url, QUrl::fromLocalFile(tempFile.fileName())); !result.success()) {
        return result;
    }

    QMimeDatabase mimeDatabase;
    const QMimeType &mime = mimeDatabase.mimeTypeForFile(tempFile.fileName());
    mimeType(mime.name());
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Mime: " << mime.name();

    totalSize(tempFile.size());
    data(tempFile.readAll());
    return KIO::WorkerResult::pass();
}

bool KioFtp::cancelTransfer(const QString &transfer)
{
    return m_kded->cancelTransfer(transfer);
}

void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    m_host = host;
    m_host = m_host.replace(QLatin1Char('-'), QLatin1Char(':')).toUpper();

    infoMessage(i18n("Connecting to the device"));

    connectToHost();
}

KIO::WorkerResult KioFtp::del(const QUrl &url, bool isfile)
{
    Q_UNUSED(isfile)

    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Del: " << url.url();

    if (auto result = changeFolder(urlDirectory(url)); !result.success()) {
        return result;
    }

    if (auto result = deleteFile(urlFileName(url)); !result.success()) {
        return result;
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::mkdir(const QUrl &url, int permissions)
{
    Q_UNUSED(permissions)

    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "MkDir: " << url.url();

    if (auto result = changeFolder(urlDirectory(url)); !result.success()) {
        return result;
    }

    if (auto result = createFolder(urlFileName(url)); !result.success()) {
        return result;
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::stat(const QUrl &url)
{
    if (auto result = testConnection(); !result.success()) {
        return result;
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Stat: " << url.url();
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Stat Dir: " << urlDirectory(url);
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Stat File: " << urlFileName(url);
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Empty Dir: " << urlDirectory(url).isEmpty();

    return statHelper(url);
}

KIO::WorkerResult KioFtp::copyHelper(const QUrl &src, const QUrl &dest)
{
    if (src.scheme() == QLatin1String("obexftp") && dest.scheme() == QLatin1String("obexftp")) {
        return copyWithinObexftp(src, dest);
    }

    if (src.scheme() == QLatin1String("obexftp")) {
        return copyFromObexftp(src, dest);
    }

    if (dest.scheme() == QLatin1String("obexftp")) {
        return copyToObexftp(src, dest);
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "This shouldn't happen...";
    return KIO::WorkerResult::fail(KIO::ERR_UNKNOWN, i18n("This should not happen"));
}

KIO::WorkerResult KioFtp::copyWithinObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Source: " << src << "Dest:" << dest;

    if (auto result = changeFolder(urlDirectory(src)); !result.success()) {
        return result;
    }

    BluezQt::PendingCall *call = m_transfer->copyFile(src.path(), dest.path());
    call->waitForFinished();

    if (call->error()) {
        // Copying files within obexftp is currently not implemented in obexd
        if (call->errorText() == QLatin1String("Not Implemented")) {
            return KIO::WorkerResult::fail(KIO::ERR_UNSUPPORTED_ACTION, src.path());
        } else {
            return KIO::WorkerResult::fail(KIO::ERR_CANNOT_WRITE, src.path());
        }
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::copyFromObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Source: " << src << "Dest:" << dest;

    if (auto result = changeFolder(urlDirectory(src)); !result.success()) {
        return result;
    }

    if (!m_statMap.contains(src.toDisplayString())) {
        auto [result, _] = listFolder(urlUpDir(src));
        if (!result.success()) {
            return result;
        }
    }

    BluezQt::PendingCall *call = m_transfer->getFile(dest.path(), urlFileName(src));
    call->waitForFinished();

    int size = m_statMap.value(src.toDisplayString()).numberValue(KIO::UDSEntry::UDS_SIZE);
    totalSize(size);

    BluezQt::ObexTransferPtr transfer = call->value().value<BluezQt::ObexTransferPtr>();
    TransferFileJob *getFile = new TransferFileJob(transfer, this);
    getFile->exec();

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::copyToObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Source:" << src << "Dest:" << dest;

    if (auto result = changeFolder(urlDirectory(dest)); !result.success()) {
        return result;
    }

    BluezQt::PendingCall *call = m_transfer->putFile(src.path(), urlFileName(dest));
    call->waitForFinished();

    int size = QFile(src.path()).size();
    totalSize(size);

    BluezQt::ObexTransferPtr transfer = call->value().value<BluezQt::ObexTransferPtr>();
    TransferFileJob *putFile = new TransferFileJob(transfer, this);
    putFile->exec();

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::statHelper(const QUrl &url)
{
    if (m_statMap.contains(url.toDisplayString())) {
        qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "statMap contains the url";
        statEntry(m_statMap.value(url.toDisplayString()));
        return KIO::WorkerResult::pass();
    }

    if (urlIsRoot(url)) {
        qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Url is root";
        KIO::UDSEntry entry;
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("/"));
        entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);

        qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Adding stat cache" << url.toDisplayString();
        m_statMap.insert(url.toDisplayString(), entry);
        statEntry(entry);
        return KIO::WorkerResult::pass();
    }

    qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "statMap does not contains the url";

    if (auto result = changeFolder(urlDirectory(url)); !result.success()) {
        return result;
    }

    auto [result, _] = listFolder(urlUpDir(url));
    if (!result.success()) {
        return result;
    }

    if (!m_statMap.contains(url.toDisplayString())) {
        qCWarning(BLUEDEVIL_KIO_OBEXFTP_LOG) << "statMap still does not contains the url!";
    }

    statEntry(m_statMap.value(url.toDisplayString()));

    return KIO::WorkerResult::pass();
}

KioFtp::ListResult KioFtp::listFolder(const QUrl &url)
{
    QList<KIO::UDSEntry> list;

    BluezQt::PendingCall *call = m_transfer->listFolder();
    call->waitForFinished();

    if (call->error()) {
        qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "List folder error" << call->errorText();
        return {KIO::WorkerResult::fail(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path()), {}};
    }

    const QList<BluezQt::ObexFileTransferEntry> &items = call->value().value<QList<BluezQt::ObexFileTransferEntry>>();

    for (const BluezQt::ObexFileTransferEntry &item : items) {
        if (!item.isValid()) {
            continue;
        }

        KIO::UDSEntry entry;
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, item.name());
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, item.label());
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, item.modificationTime().toSecsSinceEpoch());
        entry.fastInsert(KIO::UDSEntry::UDS_SIZE, item.size());

        if (item.type() == BluezQt::ObexFileTransferEntry::Folder) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        } else if (item.type() == BluezQt::ObexFileTransferEntry::File) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }

        if (urlIsRoot(url)) {
            updateRootEntryIcon(entry, item.memoryType());
        }

        list.append(entry);

        // Most probably the client of the kio will stat each file
        // so since we are on it, let's cache all of them.
        QUrl statUrl = url;

        if (statUrl.path().endsWith('/')) {
            statUrl.setPath(statUrl.path() + item.name());
        } else {
            statUrl.setPath(statUrl.path() + QLatin1Char('/') + item.name());
        }

        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(BLUEDEVIL_KIO_OBEXFTP_LOG) << "Stat:" << statUrl.toDisplayString() << entry.stringValue(KIO::UDSEntry::UDS_NAME)
                                               << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
        }
    }

    return {KIO::WorkerResult::pass(), list};
}

KIO::WorkerResult KioFtp::changeFolder(const QString &folder)
{
    BluezQt::PendingCall *call = m_transfer->changeFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_ENTER_DIRECTORY, folder);
    }
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::createFolder(const QString &folder)
{
    BluezQt::PendingCall *call = m_transfer->createFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_MKDIR, folder);
    }
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioFtp::deleteFile(const QString &file)
{
    BluezQt::PendingCall *call = m_transfer->deleteFile(file);
    call->waitForFinished();

    if (call->error()) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_DELETE, file);
    }
    return KIO::WorkerResult::pass();
}

void KioFtp::updateRootEntryIcon(KIO::UDSEntry &entry, const QString &memoryType)
{
    const QString &path = entry.stringValue(KIO::UDSEntry::UDS_NAME);

    // Nokia (mount-points are C: D: E: ...)
    if (path.size() == 2 && path.at(1) == QLatin1Char(':')) {
        if (memoryType.startsWith(QLatin1String("DEV"))) {
            entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
        } else if (memoryType == QLatin1String("MMC")) {
            entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
        }
    }

    // Android
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("PHONE_MEMORY")) {
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Phone memory"));
        entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
    } else if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("EXTERNAL_MEMORY")) {
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("External memory"));
        entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
    }
}

#include "kioobexftp.moc"

#include "moc_kioobexftp.cpp"
