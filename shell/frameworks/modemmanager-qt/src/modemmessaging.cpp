/*
    SPDX-FileCopyrightText: 2013 Anant Kamath <kamathanant@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemmessaging.h"
#include "modemmessaging_p.h"

#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif
#include "mmdebug_p.h"

#include <ModemManager/ModemManager.h>

ModemManager::ModemMessagingPrivate::ModemMessagingPrivate(const QString &path, ModemMessaging *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemMessagingIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemMessagingIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (modemMessagingIface.isValid()) {
        QList<MMSmsStorage> storages;
        Q_FOREACH (uint storage, modemMessagingIface.supportedStorages()) {
            storages.append((MMSmsStorage)storage);
        }
        supportedStorages = storages;
        defaultStorage = (MMSmsStorage)modemMessagingIface.defaultStorage();

        connect(&modemMessagingIface, &OrgFreedesktopModemManager1ModemMessagingInterface::Added, this, &ModemMessagingPrivate::onMessageAdded);
        connect(&modemMessagingIface, &OrgFreedesktopModemManager1ModemMessagingInterface::Deleted, this, &ModemMessagingPrivate::onMessageDeleted);
    }
}

ModemManager::ModemMessaging::ModemMessaging(const QString &path, QObject *parent)
    : Interface(*new ModemMessagingPrivate(path, this), parent)
{
    Q_D(ModemMessaging);
#ifdef MMQT_STATIC
    QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                          path,
                                          QLatin1String(DBUS_INTERFACE_PROPS),
                                          QStringLiteral("PropertiesChanged"),
                                          this,
                                          SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#else
    QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                         path,
                                         QLatin1String(DBUS_INTERFACE_PROPS),
                                         QStringLiteral("PropertiesChanged"),
                                         this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#endif
    QList<QDBusObjectPath> messages = d->modemMessagingIface.messages();
    Q_FOREACH (const QDBusObjectPath &op, messages) {
        const QString objectPathAsString = op.path();
        d->messageList.insert(objectPathAsString, ModemManager::Sms::Ptr());
        Q_EMIT messageAdded(objectPathAsString, false);
    }
}

ModemManager::ModemMessaging::~ModemMessaging()
{
}

ModemManager::Sms::Ptr ModemManager::ModemMessagingPrivate::findMessage(const QString &uni)
{
    ModemManager::Sms::Ptr sms;
    if (messageList.contains(uni)) {
        if (messageList.value(uni)) {
            sms = messageList.value(uni);
        } else {
            sms = ModemManager::Sms::Ptr(new ModemManager::Sms(uni), &QObject::deleteLater);
            messageList[uni] = sms;
        }
    }
    return sms;
}

ModemManager::Sms::List ModemManager::ModemMessagingPrivate::ModemMessagingPrivate::messages()
{
    ModemManager::Sms::List list;

    QMap<QString, ModemManager::Sms::Ptr>::const_iterator i;
    for (i = messageList.constBegin(); i != messageList.constEnd(); ++i) {
        ModemManager::Sms::Ptr sms = findMessage(i.key());
        if (sms) {
            list.append(sms);
        } else {
            qCWarning(MMQT) << "warning: null message for" << i.key();
        }
    }

    return list;
}

void ModemManager::ModemMessagingPrivate::onPropertiesChanged(const QString &interfaceName,
                                                              const QVariantMap &changedProperties,
                                                              const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);

    if (interfaceName == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MESSAGING)) {
        QVariantMap::const_iterator it = changedProperties.constFind(QLatin1String(MM_MODEM_MESSAGING_PROPERTY_SUPPORTEDSTORAGES));
        if (it != changedProperties.constEnd()) {
            QList<MMSmsStorage> storages;
            Q_FOREACH (uint storage, it->value<UIntList>()) {
                storages.append((MMSmsStorage)storage);
            }
            supportedStorages = storages;
        }
        it = changedProperties.constFind(QLatin1String(MM_MODEM_MESSAGING_PROPERTY_DEFAULTSTORAGE));
        if (it != changedProperties.constEnd()) {
            defaultStorage = (MMSmsStorage)it->toInt();
        }
        // MESSAGES? should be handled by Added/Deleted signals
    }
}

void ModemManager::ModemMessagingPrivate::onMessageAdded(const QDBusObjectPath &path, bool received)
{
    Q_Q(ModemMessaging);
    messageList.insert(path.path(), ModemManager::Sms::Ptr());
    Q_EMIT q->messageAdded(path.path(), received);
}

void ModemManager::ModemMessagingPrivate::onMessageDeleted(const QDBusObjectPath &path)
{
    Q_Q(ModemMessaging);
    messageList.remove(path.path());
    Q_EMIT q->messageDeleted(path.path());
}

QList<MMSmsStorage> ModemManager::ModemMessaging::supportedStorages() const
{
    Q_D(const ModemMessaging);
    return d->supportedStorages;
}

MMSmsStorage ModemManager::ModemMessaging::defaultStorage() const
{
    Q_D(const ModemMessaging);
    return d->defaultStorage;
}

ModemManager::Sms::List ModemManager::ModemMessaging::messages() const
{
    Q_D(const ModemMessaging);
    return const_cast<ModemMessagingPrivate *>(d)->messages();
}

QDBusPendingReply<QDBusObjectPath> ModemManager::ModemMessaging::createMessage(const Message &message)
{
    QVariantMap map;
    map.insert(QStringLiteral("number"), message.number);
    map.insert(QStringLiteral("text"), message.text);
    map.insert(QStringLiteral("data"), message.data);

    return createMessage(map);
}

QDBusPendingReply<QDBusObjectPath> ModemManager::ModemMessaging::createMessage(const QVariantMap &message)
{
    Q_D(ModemMessaging);

    if (!message.contains(QLatin1String("number")) || (!message.contains(QLatin1String("text")) && !message.contains(QLatin1String("data")))) {
        qCDebug(MMQT) << "Unable to create message, missing some property";
        return QDBusPendingReply<QDBusObjectPath>();
    }

    return d->modemMessagingIface.Create(message);
}

QDBusPendingReply<void> ModemManager::ModemMessaging::deleteMessage(const QString &uni)
{
    Q_D(ModemMessaging);
    return d->modemMessagingIface.Delete(QDBusObjectPath(uni));
}

ModemManager::Sms::Ptr ModemManager::ModemMessaging::findMessage(const QString &uni)
{
    Q_D(ModemMessaging);
    return d->findMessage(uni);
}

void ModemManager::ModemMessaging::setTimeout(int timeout)
{
    Q_D(ModemMessaging);
    d->modemMessagingIface.setTimeout(timeout);
}

int ModemManager::ModemMessaging::timeout() const
{
    Q_D(const ModemMessaging);
    return d->modemMessagingIface.timeout();
}

#include "moc_modemmessaging.cpp"
