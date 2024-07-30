/*
 *  SPDX-FileCopyrightText: 2010-2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
 *  SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kiobluetooth.h"
#include "filereceiversettings.h"
#include "version.h"

#include <QCoreApplication>
#include <QDBusMetaType>
#include <QThread>

#include <KLocalizedString>

#include <BluezQt/Services>

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.bluetooth" FILE "bluetooth.json")
};

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_bluetooth protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioBluetooth worker(argv[2], argv[3]);
    worker.dispatchLoop();
    return 0;
}

KioBluetooth::KioBluetooth(const QByteArray &pool, const QByteArray &app)
    : KIO::WorkerBase(QByteArrayLiteral("bluetooth"), pool, app)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    m_hasCurrentHost = false;

    Service sendFile;
    sendFile.name = i18n("Send File");
    sendFile.icon = QStringLiteral("edit-copy");
    sendFile.mimetype = QStringLiteral("application/vnd.kde.bluedevil-sendfile");
    sendFile.uuid = BluezQt::Services::ObexObjectPush;

    Service browseFiles;
    browseFiles.name = i18n("Browse Files");
    browseFiles.icon = QStringLiteral("edit-find");
    browseFiles.mimetype = QString();
    browseFiles.uuid = BluezQt::Services::ObexFileTransfer;

    m_supportedServices.insert(sendFile.uuid, sendFile);
    m_supportedServices.insert(browseFiles.uuid, browseFiles);

    qCDebug(BLUETOOTH) << "Kio Bluetooth instanced!";

    m_kded = new org::kde::BlueDevil(QStringLiteral("org.kde.kded6"), QStringLiteral("/modules/bluedevil"), QDBusConnection::sessionBus());

    if (!m_kded->isOnline()) {
        qCDebug(BLUETOOTH) << "Bluetooth is offline";
        infoMessage(i18n("No Bluetooth adapters have been found."));
        return;
    }
}

QList<KioBluetooth::Service> KioBluetooth::getSupportedServices(const QStringList &uuids)
{
    qCDebug(BLUETOOTH) << "supported services: " << uuids;

    QList<Service> retValue;
    for (const QString &uuid : uuids) {
        if (m_supportedServices.contains(uuid)) {
            retValue << m_supportedServices[uuid];
        }
    }
    return retValue;
}

KIO::WorkerResult KioBluetooth::listRemoteDeviceServices()
{
    infoMessage(i18n("Retrieving services…"));

    qCDebug(BLUETOOTH) << "Listing remote devices";

    const DeviceInfo &info = m_kded->device(m_currentHostAddress).value();
    if (info.isEmpty()) {
        qCDebug(BLUETOOTH) << "Invalid hostname!";
        infoMessage(i18n("This address is unavailable."));
        return KIO::WorkerResult::pass();
    }

    const QList<Service> &services = getSupportedServices(info.value(QStringLiteral("UUIDs")).split(QLatin1Char(',')));

    qCDebug(BLUETOOTH) << "Num of supported services: " << services.size();

    int i = 1;
    totalSize(services.count());

    for (const Service &service : services) {
        KIO::UDSEntry entry;
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, service.uuid);
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, service.name);
        entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, service.icon);
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);

        // If it is browse files, act as a folder
        if (service.uuid == BluezQt::Services::ObexFileTransfer) {
            QUrl obexUrl;
            obexUrl.setScheme(QStringLiteral("obexftp"));
            obexUrl.setHost(m_currentHostname.replace(QLatin1Char(':'), QLatin1Char('-')).toUpper());
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.fastInsert(KIO::UDSEntry::UDS_URL, obexUrl.toString());
        } else {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }

        if (service.mimetype.isEmpty()) {
            entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/vnd.kde.bluedevil.service"));
        } else {
            entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, service.mimetype);
        }

        listEntry(entry);
        processedSize(i++);
    }

    infoMessage(QString());
    return KIO::WorkerResult::pass();
}

void KioBluetooth::listDownload()
{
    KIO::UDSEntry entry;
    entry.clear();
    entry.fastInsert(KIO::UDSEntry::UDS_URL, FileReceiverSettings::saveUrl().toDisplayString());
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, i18n("Received Files"));
    entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("folder-downloads"));
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    listEntry(entry);
}

void KioBluetooth::listDevices()
{
    qCDebug(BLUETOOTH) << "Asking kded for devices";
    const QMapDeviceInfo &devices = m_kded->allDevices().value();
    qCDebug(BLUETOOTH) << devices.keys();

    for (const DeviceInfo &device : devices) {
        listDevice(device);
    }

    m_kded->startDiscovering(10 * 1000);

    infoMessage(i18n("Scanning for new devices…"));
}

void KioBluetooth::listDevice(const DeviceInfo device)
{
    qCDebug(BLUETOOTH) << device;
    if (getSupportedServices(device[QStringLiteral("UUIDs")].split(QStringLiteral(","))).isEmpty()) {
        return;
    }
    QString target = QStringLiteral("bluetooth://");
    target.append(QString(device[QStringLiteral("address")]).replace(QLatin1Char(':'), QLatin1Char('-')) + QLatin1Char('/'));

    KIO::UDSEntry entry;
    entry.fastInsert(KIO::UDSEntry::UDS_URL, target);
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, device[QStringLiteral("name")]);
    entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, device[QStringLiteral("icon")]);
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/vnd.kde.bluedevil.device"));
    listEntry(entry);
}

KIO::WorkerResult KioBluetooth::listDir(const QUrl &url)
{
    qCDebug(BLUETOOTH) << "Listing..." << url;

    // Url is not used here because all we could care about the url is the host, and that's already
    // handled in @p setHost
    Q_UNUSED(url);

    // If we are not online (ie. there's no working bluetooth adapter), list an empty dir
    qCDebug(BLUETOOTH) << m_kded->isOnline().value();
    if (!m_kded->isOnline().value()) {
        infoMessage(i18n("No Bluetooth adapters have been found."));
        return KIO::WorkerResult::pass();
    }

    if (!m_hasCurrentHost) {
        listDownload();
        listDevices();
        return KIO::WorkerResult::pass();
    } else {
        return listRemoteDeviceServices();
    }
}

KIO::WorkerResult KioBluetooth::stat(const QUrl &url)
{
    qCDebug(BLUETOOTH) << "Stat: " << url;
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult KioBluetooth::get(const QUrl &url)
{
    m_kded->stopDiscovering();
    qCDebug(BLUETOOTH) << "Get: " << url;
    qCDebug(BLUETOOTH) << m_supportedServices.value(url.fileName()).mimetype;
    mimeType(m_supportedServices.value(url.fileName()).mimetype);
    return KIO::WorkerResult::pass();
}

void KioBluetooth::setHost(const QString &hostname, quint16 port, const QString &user, const QString &pass)
{
    qCDebug(BLUETOOTH) << "Setting host: " << hostname;

    // In this kio only the hostname (constHostname) is used
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    if (hostname.isEmpty()) {
        m_hasCurrentHost = false;
    } else {
        m_hasCurrentHost = true;

        m_currentHostname = hostname;
        m_currentHostAddress = hostname.toUpper();
        m_currentHostAddress.replace(QLatin1Char('-'), QLatin1Char(':'));
    }
}

Q_LOGGING_CATEGORY(BLUETOOTH, "bluedevil.kio_bluetooth")

#include "kiobluetooth.moc"

#include "moc_kiobluetooth.cpp"
