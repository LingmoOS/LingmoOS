/*
    SPDX-FileCopyrightText: 2013, 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemdevice.h"
#include "modemdevice_p.h"

#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif
#include "manager.h"
#include "mmdebug_p.h"
#include "modem3gpp.h"
#include "modem3gppussd.h"
#include "modemcdma.h"
#include "modemlocation.h"
#if MM_CHECK_VERSION(1, 2, 0)
#include "modemoma.h"
#include "modemsignal.h"
#endif
#if MM_CHECK_VERSION(1, 6, 0)
#include "modemvoice.h"
#endif
#include "modemfirmware.h"
#include "modemsimple.h"
#include "modemtime.h"

#include <QDBusConnection>
#include <QDomDocument>

ModemManager::ModemDevicePrivate::ModemDevicePrivate(const QString &path, ModemManager::ModemDevice *q)
    : uni(path)
    , q_ptr(q)
{
    init();
}

ModemManager::ModemDevicePrivate::~ModemDevicePrivate()
{
}

void ModemManager::ModemDevicePrivate::init()
{
    Q_Q(ModemManager::ModemDevice);
#ifdef MMQT_STATIC
    bool success = QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                                         QLatin1String(MMQT_DBUS_PATH),
                                                         QLatin1String(DBUS_INTERFACE_MANAGER),
                                                         QStringLiteral("InterfacesAdded"),
                                                         this,
                                                         SLOT(onInterfacesAdded(QDBusObjectPath, ModemManager::MMVariantMapMap)));
    success = QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                                    QLatin1String(MMQT_DBUS_PATH),
                                                    QLatin1String(DBUS_INTERFACE_MANAGER),
                                                    QStringLiteral("InterfacesRemoved"),
                                                    this,
                                                    SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));
#else
    bool success = QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                                        QLatin1String(MMQT_DBUS_PATH),
                                                        QLatin1String(DBUS_INTERFACE_MANAGER),
                                                        QStringLiteral("InterfacesAdded"),
                                                        this,
                                                        SLOT(onInterfacesAdded(QDBusObjectPath, ModemManager::MMVariantMapMap)));
    success = QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                                   QLatin1String(MMQT_DBUS_PATH),
                                                   QLatin1String(DBUS_INTERFACE_MANAGER),
                                                   QStringLiteral("InterfacesRemoved"),
                                                   this,
                                                   SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));
#endif

    initInterfaces();
}

void ModemManager::ModemDevicePrivate::initInterfaces()
{
    interfaceList.clear();

    const QString xmlData = introspect();
    if (xmlData.isEmpty()) {
        qCDebug(MMQT) << uni << "has no interfaces!";
        return;
    }

    QDomDocument dom;
    dom.setContent(xmlData);

    QDomNodeList ifaceNodeList = dom.elementsByTagName(QStringLiteral("interface"));
    for (int i = 0; i < ifaceNodeList.count(); i++) {
        QDomElement ifaceElem = ifaceNodeList.item(i).toElement();
        /* Accept only MM interfaces so that when the device is unplugged,
         * interfaceList goes empty and we can easily verify that the device is gone. */
        if (!ifaceElem.isNull() && ifaceElem.attribute(QStringLiteral("name")).startsWith(QLatin1String(MMQT_DBUS_SERVICE))) {
            const QString name = ifaceElem.attribute(QStringLiteral("name"));
            if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM)) {
                interfaceList.insert(ModemManager::ModemDevice::ModemInterface, ModemManager::Modem::Ptr());
                if (interfaceList.contains(ModemManager::ModemDevice::ModemInterface)) {
                    ModemManager::Modem::Ptr modemInterface = interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                    if (!modemInterface->simPath().isEmpty()) {
                        simCard = ModemManager::Sim::Ptr(new ModemManager::Sim(modemInterface->simPath()), &QObject::deleteLater);
                        // q->simAdded(modemInterface->simPath());

                        QObject::connect(modemInterface.data(), SIGNAL(simPathChanged(QString, QString)), this, SLOT(onSimPathChanged(QString, QString)));
                    }
                }
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIMPLE)) {
                interfaceList.insert(ModemManager::ModemDevice::SimpleInterface, ModemManager::ModemSimple::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP)) {
                interfaceList.insert(ModemManager::ModemDevice::GsmInterface, ModemManager::Modem3gpp::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD)) {
                interfaceList.insert(ModemManager::ModemDevice::GsmUssdInterface, ModemManager::Modem3gppUssd::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA)) {
                interfaceList.insert(ModemManager::ModemDevice::CdmaInterface, ModemManager::ModemCdma::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MESSAGING)) {
                interfaceList.insert(ModemManager::ModemDevice::MessagingInterface, ModemManager::ModemMessaging::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_LOCATION)) {
                interfaceList.insert(ModemManager::ModemDevice::LocationInterface, ModemManager::ModemLocation::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_TIME)) {
                interfaceList.insert(ModemManager::ModemDevice::TimeInterface, ModemManager::ModemTime::Ptr());
            }
#if MM_CHECK_VERSION(1, 2, 0)
            else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_OMA)) {
                interfaceList.insert(ModemManager::ModemDevice::OmaInterface, ModemManager::ModemOma::Ptr());
            } else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIGNAL)) {
                interfaceList.insert(ModemManager::ModemDevice::SignalInterface, ModemManager::ModemSignal::Ptr());
            }
#endif
            else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_FIRMWARE)) {
                interfaceList.insert(ModemManager::ModemDevice::FirmwareInterface, ModemManager::ModemFirmware::Ptr());
            }
#if MM_CHECK_VERSION(1, 6, 0)
            else if (name == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_VOICE)) {
                interfaceList.insert(ModemManager::ModemDevice::VoiceInterface, ModemManager::ModemVoice::Ptr());
            }
#endif
        }
    }

    qCDebug(MMQT) << uni << "has interfaces:" << interfaceList.keys();
}

QString ModemManager::ModemDevicePrivate::introspect() const
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(QLatin1String(MMQT_DBUS_SERVICE), uni, QLatin1String(DBUS_INTERFACE_INTROSPECT), QLatin1String("Introspect"));
#ifdef MMQT_STATIC
    QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(call);
#else
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);
#endif

    if (reply.isValid()) {
        return reply.value();
    } else {
        return QString();
    }
}

ModemManager::ModemDevice::ModemDevice(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new ModemDevicePrivate(path, this))
{
}

ModemManager::ModemDevice::ModemDevice(ModemDevicePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

ModemManager::ModemDevice::~ModemDevice()
{
    delete d_ptr;
}

ModemManager::Interface::List ModemManager::ModemDevicePrivate::interfaces()
{
    ModemManager::Interface::List list;

    QMap<ModemManager::ModemDevice::InterfaceType, ModemManager::Interface::Ptr>::const_iterator i;
    for (i = interfaceList.constBegin(); i != interfaceList.constEnd(); ++i) {
        ModemManager::Interface::Ptr modemInterface = interface(i.key());
        if (modemInterface) {
            list.append(modemInterface);
        } else {
            qCWarning(MMQT) << "warning: null network Interface for" << i.key();
        }
    }

    return list;
}

ModemManager::Interface::Ptr ModemManager::ModemDevicePrivate::interface(ModemManager::ModemDevice::InterfaceType type)
{
    ModemManager::Interface::Ptr modemInterface;

    if (interfaceList.contains(type)) {
        if (interfaceList.value(type)) {
            modemInterface = interfaceList.value(type);
        } else {
            modemInterface = createInterface(type);
            interfaceList[type] = modemInterface;
        }
    }

    return modemInterface;
}

ModemManager::Interface::Ptr ModemManager::ModemDevicePrivate::createInterface(ModemManager::ModemDevice::InterfaceType type)
{
    ModemManager::Interface::Ptr createdInterface;
    switch (type) {
    case ModemManager::ModemDevice::ModemInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::Modem(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::SimpleInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemSimple(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::GsmInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::Modem3gpp(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::GsmUssdInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::Modem3gppUssd(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::CdmaInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemCdma(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::MessagingInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemMessaging(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::LocationInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemLocation(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::TimeInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemTime(uni), &QObject::deleteLater);
        break;
#if MM_CHECK_VERSION(1, 2, 0)
    case ModemManager::ModemDevice::OmaInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemOma(uni), &QObject::deleteLater);
        break;
    case ModemManager::ModemDevice::SignalInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemSignal(uni), &QObject::deleteLater);
        break;
#endif
    case ModemManager::ModemDevice::FirmwareInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemFirmware(uni), &QObject::deleteLater);
        break;
#if MM_CHECK_VERSION(1, 6, 0)
    case ModemManager::ModemDevice::VoiceInterface:
        createdInterface = ModemManager::Interface::Ptr(new ModemManager::ModemVoice(uni), &QObject::deleteLater);
        break;
#endif
    }
    return createdInterface;
}

ModemManager::Bearer::Ptr ModemManager::ModemDevicePrivate::findBearer(const QString &uni)
{
    ModemManager::Bearer::Ptr result;
    Q_FOREACH (const ModemManager::Bearer::Ptr &bearer, bearers()) {
        if (bearer->uni() == uni) {
            result = bearer;
            break;
        }
    }

    return result;
}

ModemManager::Bearer::List ModemManager::ModemDevicePrivate::bearers()
{
    Q_Q(ModemManager::ModemDevice);

    return q->modemInterface()->listBearers();
}

ModemManager::Sim::Ptr ModemManager::ModemDevicePrivate::sim()
{
    return simCard;
}

QString ModemManager::ModemDevice::uni() const
{
    Q_D(const ModemDevice);
    return d->uni;
}

bool ModemManager::ModemDevice::hasInterface(ModemManager::ModemDevice::InterfaceType type) const
{
    Q_D(const ModemDevice);
    return d->interfaceList.contains(type);
}

ModemManager::Interface::Ptr ModemManager::ModemDevice::interface(ModemManager::ModemDevice::InterfaceType type) const
{
    Q_D(const ModemDevice);

    return const_cast<ModemDevicePrivate *>(d)->interface(type);
}

ModemManager::Interface::List ModemManager::ModemDevice::interfaces() const
{
    Q_D(const ModemDevice);

    return const_cast<ModemDevicePrivate *>(d)->interfaces();
}

ModemManager::ModemMessaging::Ptr ModemManager::ModemDevice::messagingInterface()
{
    Q_D(ModemDevice);

    return d->interface(ModemManager::ModemDevice::MessagingInterface).objectCast<ModemManager::ModemMessaging>();
}

ModemManager::Modem::Ptr ModemManager::ModemDevice::modemInterface()
{
    Q_D(ModemDevice);

    return d->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
}

ModemManager::Bearer::Ptr ModemManager::ModemDevice::findBearer(const QString &uni)
{
    Q_D(ModemDevice);

    return d->findBearer(uni);
}

ModemManager::Bearer::List ModemManager::ModemDevice::bearers() const
{
    Q_D(const ModemDevice);

    return const_cast<ModemDevicePrivate *>(d)->bearers();
}

ModemManager::Sim::Ptr ModemManager::ModemDevice::sim() const
{
    Q_D(const ModemDevice);

    return const_cast<ModemDevicePrivate *>(d)->sim();
}

bool ModemManager::ModemDevice::isGsmModem() const
{
    return hasInterface(ModemManager::ModemDevice::GsmInterface);
}

bool ModemManager::ModemDevice::isCdmaModem() const
{
    return hasInterface(ModemManager::ModemDevice::CdmaInterface);
}

void ModemManager::ModemDevicePrivate::onInterfacesAdded(const QDBusObjectPath &object_path, const MMVariantMapMap &interfaces_and_properties)
{
    Q_Q(ModemDevice);
    if (object_path.path() != uni) {
        return;
    }

    Q_FOREACH (const QString &iface, interfaces_and_properties.keys()) {
        /* Don't store generic DBus interfaces */
        if (iface.startsWith(QLatin1String(MMQT_DBUS_SERVICE))) {
            if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM)) {
                interfaceList.insert(ModemManager::ModemDevice::ModemInterface, ModemManager::Modem::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::ModemInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIMPLE)) {
                interfaceList.insert(ModemManager::ModemDevice::SimpleInterface, ModemManager::ModemSimple::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::SimpleInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP)) {
                interfaceList.insert(ModemManager::ModemDevice::GsmInterface, ModemManager::Modem3gpp::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::GsmInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD)) {
                interfaceList.insert(ModemManager::ModemDevice::GsmUssdInterface, ModemManager::Modem3gppUssd::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::GsmUssdInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA)) {
                interfaceList.insert(ModemManager::ModemDevice::CdmaInterface, ModemManager::ModemCdma::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::CdmaInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MESSAGING)) {
                interfaceList.insert(ModemManager::ModemDevice::MessagingInterface, ModemManager::ModemMessaging::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::MessagingInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_LOCATION)) {
                interfaceList.insert(ModemManager::ModemDevice::LocationInterface, ModemManager::ModemLocation::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::LocationInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_TIME)) {
                interfaceList.insert(ModemManager::ModemDevice::TimeInterface, ModemManager::ModemTime::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::TimeInterface);
            }
#if MM_CHECK_VERSION(1, 2, 0)
            else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_OMA)) {
                interfaceList.insert(ModemManager::ModemDevice::OmaInterface, ModemManager::ModemOma::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::OmaInterface);
            } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIGNAL)) {
                interfaceList.insert(ModemManager::ModemDevice::SignalInterface, ModemManager::ModemSignal::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::SignalInterface);
            }
#endif
            else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_FIRMWARE)) {
                interfaceList.insert(ModemManager::ModemDevice::FirmwareInterface, ModemManager::ModemFirmware::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::FirmwareInterface);
            }
#if MM_CHECK_VERSION(1, 6, 0)
            else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_VOICE)) {
                interfaceList.insert(ModemManager::ModemDevice::VoiceInterface, ModemManager::ModemVoice::Ptr());
                Q_EMIT q->interfaceAdded(ModemManager::ModemDevice::VoiceInterface);
            }
#endif
        }
    }
}

void ModemManager::ModemDevicePrivate::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    Q_Q(ModemDevice);
    if (object_path.path() != uni) {
        return;
    }

    if (interfaces.contains(QLatin1String(MMQT_DBUS_INTERFACE_MODEM)) || interfaces.isEmpty()) {
        if (simCard) {
            Q_EMIT q->simRemoved(simCard->uni());
            simCard = ModemManager::Sim::Ptr();
        }
    }

    Q_FOREACH (const QString &iface, interfaces) {
        if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM)) {
            interfaceList.remove(ModemManager::ModemDevice::ModemInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::ModemInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIMPLE)) {
            interfaceList.remove(ModemManager::ModemDevice::SimpleInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::SimpleInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP)) {
            interfaceList.remove(ModemManager::ModemDevice::GsmInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::GsmInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD)) {
            interfaceList.remove(ModemManager::ModemDevice::GsmUssdInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::GsmUssdInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA)) {
            interfaceList.remove(ModemManager::ModemDevice::CdmaInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::CdmaInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MESSAGING)) {
            interfaceList.remove(ModemManager::ModemDevice::MessagingInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::MessagingInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_LOCATION)) {
            interfaceList.remove(ModemManager::ModemDevice::LocationInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::LocationInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_TIME)) {
            interfaceList.remove(ModemManager::ModemDevice::TimeInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::TimeInterface);
        }
#if MM_CHECK_VERSION(1, 2, 0)
        else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_OMA)) {
            interfaceList.remove(ModemManager::ModemDevice::OmaInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::OmaInterface);
        } else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_SIGNAL)) {
            interfaceList.remove(ModemManager::ModemDevice::SignalInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::SignalInterface);
        }
#endif
        else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_FIRMWARE)) {
            interfaceList.remove(ModemManager::ModemDevice::FirmwareInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::FirmwareInterface);
        }
#if MM_CHECK_VERSION(1, 6, 0)
        else if (iface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_VOICE)) {
            interfaceList.remove(ModemManager::ModemDevice::VoiceInterface);
            Q_EMIT q->interfaceRemoved(ModemManager::ModemDevice::VoiceInterface);
        }
#endif
    }
}

void ModemManager::ModemDevicePrivate::onSimPathChanged(const QString &oldPath, const QString &newPath)
{
    Q_Q(ModemDevice);

    if (!oldPath.isEmpty() && simCard->uni() == oldPath) {
        Q_EMIT q->simRemoved(oldPath);
        simCard = ModemManager::Sim::Ptr();
    }

    if (!newPath.isEmpty()) {
        simCard = ModemManager::Sim::Ptr(new ModemManager::Sim(newPath), &QObject::deleteLater);
        Q_EMIT q->simAdded(newPath);
    }
}

#include "moc_modemdevice.cpp"
#include "moc_modemdevice_p.cpp"
