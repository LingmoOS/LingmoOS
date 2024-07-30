/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DevicesModel.h"

#include <KCupsPrinter.h>
#include <kcupslib_log.h>

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QHostInfo>

using namespace Qt::StringLiterals;

DevicesModel::DevicesModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_request(nullptr)
    , m_rx(QLatin1String("[a-z]+://.*"))
    , m_blacklistedURIs({QLatin1String("hp"),
                         QLatin1String("hpfax"),
                         QLatin1String("hal"),
                         QLatin1String("beh"),
                         QLatin1String("scsi"),
                         QLatin1String("http"),
                         QLatin1String("delete")})
{
    m_roles = QStandardItemModel::roleNames();
    m_roles[DeviceClass] = "deviceClass";
    m_roles[DeviceId] = "deviceId";
    m_roles[DeviceInfo] = "deviceInfo";
    m_roles[DeviceMakeAndModel] = "deviceMakeModel";
    m_roles[DeviceUri] = "deviceUri";
    m_roles[DeviceUris] = "deviceUris";
    m_roles[DeviceLocation] = "deviceLocation";
    m_roles[DeviceDescription] = "deviceDescription";
    m_roles[DeviceCategory] = "deviceCategory";

    qDBusRegisterMetaType<MapSS>();
    qDBusRegisterMetaType<MapSMapSS>();

    // Adds the other device which is meant for manual URI input
    insertDevice(QLatin1String("other"), QString(), i18nc("@item", "Manual Config"), QString(), QLatin1String("other"), QString());
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return m_roles;
}

QString DevicesModel::uriDevice(const QString &uri) const
{
    QString ret;
    if (uri.startsWith(QLatin1String("parallel"))) {
        ret = i18n("Parallel Port");
    } else if (uri.startsWith(QLatin1String("serial"))) {
        ret = i18n("Serial Port");
    } else if (uri.startsWith(QLatin1String("usb"))) {
        ret = i18n("USB");
    } else if (uri.startsWith(QLatin1String("bluetooth"))) {
        ret = i18n("Bluetooth");
    } else if (uri.startsWith(QLatin1String("hpfax"))) {
        ret = i18n("Fax - HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hp"))) {
        ret = i18n("HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hal"))) {
        ret = i18n("Hardware Abstraction Layer (HAL)");
    } else if (uri.startsWith(QLatin1String("socket"))) {
        ret = i18n("AppSocket/HP JetDirect");
    } else if (uri.startsWith(QLatin1String("lpd"))) {
        // Check if the queue name is defined
        const QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("LPD/LPR queue");
        } else {
            ret = i18n("LPD/LPR queue %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("smb"))) {
        ret = i18n("Windows Printer via SAMBA");
    } else if (uri.startsWith(QLatin1String("ipp"))) {
        // Check if the queue name (fileName) is defined
        const QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("IPP");
        } else {
            ret = i18n("IPP %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("https"))) {
        ret = i18n("HTTP");
    } else if (uri.startsWith(QLatin1String("dnssd")) || uri.startsWith(QLatin1String("mdns"))) {
        // TODO this needs testing...
        QString text;
        if (uri.contains(QLatin1String("cups"))) {
            text = i18n("Remote CUPS printer via DNS-SD");
        } else {
            if (uri.contains(QLatin1String("._ipp"))) {
                ret = i18n("IPP network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._printer"))) {
                ret = i18n("LPD network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._pdl-datastream"))) {
                ret = i18n("AppSocket/JetDirect network printer via DNS-SD");
            } else {
                ret = i18n("Network printer via DNS-SD");
            }
        }
    } else {
        ret = uri;
    }
    return ret;
}

QString DevicesModel::deviceDescription(const QString &uri) const
{
    static QMap<QString, QString> descriptions(
        {{u"parallel"_s, i18nc("@info:tooltip", "A printer connected to the parallel port")},
         {u"bluetooth"_s, i18nc("@info:tooltip", "A printer connected via Bluetooth")},
         {u"hal"_s, i18nc("@info:tooltip", "Local printer detected by the Hardware Abstraction Layer (HAL)")},
         {u"hpfax"_s, i18nc("@info:tooltip", "HPLIP software driving a fax machine,\nor the fax function of a multi-function device")},
         {u"hp"_s, i18nc("@info:tooltip", "HPLIP software driving a printer,\nor the printer function of a multi-function device")},
         {u"ipp"_s, i18nc("@info:tooltip", "IPP Network printer via IPP")},
         {u"usb"_s, i18nc("@info:tooltip", "A printer connected to a USB port")}});

    QString ret;

    if (uri.startsWith(u"dnssd"_s) || uri.startsWith(u"mdns"_s)) {
        if (uri.contains(u"cups"_s)) {
            ret = i18nc("@info:tooltip", "Remote CUPS printer via DNS-SD");
        } else {
            QString protocol;
            if (uri.contains(u"._ipp"_s)) {
                protocol = u"IPP"_s;
            } else if (uri.contains(u"._printer"_s)) {
                protocol = u"LPD"_s;
            } else if (uri.contains(u"._pdl-datastream"_s)) {
                protocol = u"AppSocket/JetDirect"_s;
            }

            if (protocol.isEmpty()) {
                ret = i18nc("@info:tooltip", "Network printer via DNS-SD");
            } else {
                ret = i18nc("@info:tooltip", "%1 network printer via DNS-SD", protocol);
            }
        }
    } else {
        for (auto i = descriptions.cbegin(), end = descriptions.cend(); i != end; ++i) {
            if (uri.startsWith(i.key())) {
                return i.value();
            }
        }
    }

    return ret.isEmpty() ? uri : ret;
}

void DevicesModel::update()
{
    if (m_request) {
        return;
    }

    // clear the model to don't duplicate items
    if (rowCount()) {
        removeRows(1, rowCount() - 1);
    }
    m_request = new KCupsRequest;
    connect(m_request, &KCupsRequest::device, this, &DevicesModel::gotDevice);
    connect(m_request, &KCupsRequest::finished, this, &DevicesModel::finished);

    // Get devices with 5 seconds of timeout
    m_request->getDevices(10);
}

void DevicesModel::gotDevice(const QString &device_class,
                             const QString &device_id,
                             const QString &device_info,
                             const QString &device_make_and_model,
                             const QString &device_uri,
                             const QString &device_location)
{
    // "direct"
    qCDebug(LIBKCUPS) << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    qCDebug(LIBKCUPS) << device_id;
    // "Samsung SCX-4200 Series"
    qCDebug(LIBKCUPS) << device_info;
    // "Samsung SCX-4200 Series"
    qCDebug(LIBKCUPS) << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    qCDebug(LIBKCUPS) << device_uri;
    // ""
    qCDebug(LIBKCUPS) << device_location;

    if (m_blacklistedURIs.contains(device_uri)) {
        // ignore black listed uri's
        return;
    }

    // For the protocols, not real devices
    if (device_id.isEmpty() && device_make_and_model == QLatin1String("Unknown")) {
        insertDevice(device_class, device_id, device_info, device_make_and_model, device_uri, device_location);
    } else {
        // Map the devices so later we try to group them
        const MapSS mapSS({{KCUPS_DEVICE_CLASS, device_class},
                           {KCUPS_DEVICE_ID, device_id},
                           {KCUPS_DEVICE_INFO, device_info},
                           {KCUPS_DEVICE_MAKE_AND_MODEL, device_make_and_model},
                           {KCUPS_DEVICE_LOCATION, device_location}});
        m_mappedDevices[device_uri] = mapSS;
    }
}

void DevicesModel::finished()
{
    bool hasError = m_request->hasError();
    if (hasError) {
        Q_EMIT errorMessage(i18n("Failed to get a list of devices: '%1'", m_request->errorMsg()));
    }
    m_request->deleteLater();
    m_request = nullptr;

    if (hasError || m_mappedDevices.isEmpty()) {
        Q_EMIT loaded();
        return;
    }

    // Try to group the physical devices
    auto call = QDBusMessage::createMethodCall(u"org.fedoraproject.Config.Printing"_s, u"/org/fedoraproject/Config/Printing"_s, u"org.fedoraproject.Config.Printing"_s, u"GroupPhysicalDevices"_s);
    call.setArguments({QVariant::fromValue(m_mappedDevices)});
    const auto pending = QDBusConnection::sessionBus().asyncCall(call);
    const auto watcher = new QDBusPendingCallWatcher(pending, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QList<QStringList>> reply(*w);

        if (reply.isError()) {
            qCWarning(LIBKCUPS) << reply.error().name() << reply.error().message();
            // Fallback, insert all the devices
            MapSMapSS::const_iterator i = m_mappedDevices.constBegin();
            while (i != m_mappedDevices.constEnd()) {
                const MapSS device = i.value();
                insertDevice(device[KCUPS_DEVICE_CLASS],
                             device[KCUPS_DEVICE_ID],
                             device[KCUPS_DEVICE_INFO],
                             device[KCUPS_DEVICE_MAKE_AND_MODEL],
                             i.key(),
                             device[KCUPS_DEVICE_LOCATION]);
                ++i;
            }
            Q_EMIT errorMessage(i18n("Failed to group devices: '%1'", reply.error().message()));
        } else {
            const auto groupedDevices = reply.value();
            for (const auto &list : groupedDevices) {
                if (list.isEmpty()) {
                    continue;
                }

                const QString uri = list.first();
                const MapSS device = m_mappedDevices[uri];
                insertDevice(device[KCUPS_DEVICE_CLASS],
                             device[KCUPS_DEVICE_ID],
                             device[KCUPS_DEVICE_INFO],
                             device[KCUPS_DEVICE_MAKE_AND_MODEL],
                             uri,
                             device[KCUPS_DEVICE_LOCATION],
                             list.size() > 1 ? list : QStringList());
            }
        }

        Q_EMIT loaded();
        w->deleteLater();
    });
}

void DevicesModel::insertDevice(const QString &device_class,
                                const QString &device_id,
                                const QString &device_info,
                                const QString &device_make_and_model,
                                const QString &device_uri,
                                const QString &device_location,
                                const QStringList &grouped_uris)
{
    QStandardItem *stdItem;
    stdItem = createItem(device_class, device_id, device_info, device_make_and_model, device_uri, device_location, !grouped_uris.isEmpty());
    if (!grouped_uris.isEmpty()) {
        stdItem->setData(grouped_uris, DeviceUris);
    }
}

void DevicesModel::insertDevice(const QString &device_class,
                                const QString &device_id,
                                const QString &device_info,
                                const QString &device_make_and_model,
                                const QString &device_uri,
                                const QString &device_location,
                                const KCupsPrinters &grouped_printers)
{
    QStandardItem *stdItem;
    stdItem = createItem(device_class, device_id, device_info, device_make_and_model, device_uri, device_location, !grouped_printers.isEmpty());
    if (!grouped_printers.isEmpty()) {
        stdItem->setData(QVariant::fromValue(grouped_printers), DeviceUris);
    }
}

QStandardItem *DevicesModel::createItem(const QString &device_class,
                                        const QString &device_id,
                                        const QString &device_info,
                                        const QString &device_make_and_model,
                                        const QString &device_uri,
                                        const QString &device_location,
                                        bool grouped)
{
    // "direct"
    qCDebug(LIBKCUPS) << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    qCDebug(LIBKCUPS) << device_id;
    // "Samsung SCX-4200 Series"
    qCDebug(LIBKCUPS) << device_info;
    // "Samsung SCX-4200 Series"
    qCDebug(LIBKCUPS) << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    qCDebug(LIBKCUPS) << device_uri;
    // ""
    qCDebug(LIBKCUPS) << device_location;

    Kind kind;
    // Store the kind of the device
    if (device_class == QLatin1String("network")) {
        const auto match = m_rx.match(device_uri);
        if (match.hasMatch()) {
            kind = Networked;
        } else {
            // other network devices looks like
            // just "http"
            kind = OtherNetworked;
        }
    } else if (device_class == QLatin1String("other") && device_uri == QLatin1String("other")) {
        kind = Other;
    } else {
        // If device class is not network assume local
        kind = Local;
    }

    QString location;
    if (device_location.isEmpty() && kind == Local) {
        location = QHostInfo::localHostName();
    } else {
        location = device_location;
    }

    QString text;
    if (!device_make_and_model.isEmpty() && !grouped && device_make_and_model.compare(QLatin1String("unknown"), Qt::CaseInsensitive)) {
        text = device_info + QLatin1String(" (") + device_make_and_model + QLatin1Char(')');
    } else {
        text = device_info;
    }

    QString toolTip;
    if (!grouped) {
        toolTip = deviceDescription(device_uri);
    }

    auto stdItem = new QStandardItem;
    stdItem->setText(text);
    stdItem->setToolTip(toolTip);
    stdItem->setData(device_class, DeviceClass);
    stdItem->setData(device_id, DeviceId);
    stdItem->setData(device_info, DeviceInfo);
    stdItem->setData(device_uri, DeviceUri);
    stdItem->setData(device_make_and_model, DeviceMakeAndModel);
    stdItem->setData(device_location, DeviceLocation);
    stdItem->setData(deviceDescription(device_uri), DeviceDescription);

    // Find the proper category to our item
    QStandardItem *catItem;
    QString cat;
    switch (kind) {
    case Networked:
        cat = i18nc("@item", "Discovered Network Printers");
        catItem = findCreateCategory(cat, kind);
        stdItem->setData(u"Network"_s, DeviceCategory);
        catItem->appendRow(stdItem);
        break;
    case OtherNetworked:
        cat = i18nc("@item", "Other Network Printers");
        catItem = findCreateCategory(cat, kind);
        stdItem->setData(u"Manual"_s, DeviceCategory);
        catItem->appendRow(stdItem);
        break;
    case Local:
        cat = i18nc("@item", "Local Printers");
        catItem = findCreateCategory(cat, kind);
        stdItem->setData(u"Local"_s, DeviceCategory);
        catItem->appendRow(stdItem);
        break;
    default:
        stdItem->setData(kind, Qt::UserRole);
        stdItem->setData(u"Manual"_s, DeviceCategory);
        appendRow(stdItem);
    }

    return stdItem;
}

QStandardItem *DevicesModel::findCreateCategory(const QString &category, Kind kind)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *catItem = item(i);
        if (catItem->data(Qt::UserRole).toInt() == kind) {
            return catItem;
        }
    }

    int pos = 0;
    for (int i = 0; i < rowCount(); ++i, ++pos) {
        QStandardItem *catItem = item(i);
        if (catItem->data(Qt::UserRole).toInt() > kind) {
            pos = i;
            break;
        }
    }

    auto catItem = new QStandardItem(category);
    QFont font = catItem->font();
    font.setBold(true);
    catItem->setFont(font);
    catItem->setData(kind, Qt::UserRole);
    catItem->setFlags(Qt::ItemIsEnabled);
    insertRow(pos, catItem);

    // Emit the parent so the view expand the item
    Q_EMIT parentAdded(indexFromItem(catItem));

    return catItem;
}

#include "moc_DevicesModel.cpp"
