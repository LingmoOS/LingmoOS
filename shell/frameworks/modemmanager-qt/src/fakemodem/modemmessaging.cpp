/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemmessaging.h"

#include <QDBusConnection>
#include <QDBusMessage>

ModemMessaging::ModemMessaging(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
    , m_messageCounter(0)
    , m_defaultStorage(0)
{
}

ModemMessaging::~ModemMessaging()
{
}

uint ModemMessaging::defaultStorage() const
{
    return m_defaultStorage;
}

QList<QDBusObjectPath> ModemMessaging::messages() const
{
    return m_messages.keys();
}

ModemManager::UIntList ModemMessaging::supportedStorages() const
{
    return m_supportedStorages;
}

QDBusObjectPath ModemMessaging::Create(const QVariantMap &properties)
{
    Q_UNUSED(properties);
    return QDBusObjectPath();
}

void ModemMessaging::Delete(const QDBusObjectPath &path)
{
    m_messages.remove(path);
    QDBusConnection::sessionBus().unregisterObject(path.path());

    Q_EMIT Deleted(path);
}

QList<QDBusObjectPath> ModemMessaging::List()
{
    return m_messages.keys();
}

void ModemMessaging::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void ModemMessaging::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void ModemMessaging::addMessage(Sms *sms)
{
    QString newSmsPath = QStringLiteral("/org/kde/fakemodem/Sms/") + QString::number(m_messageCounter++);
    sms->setSmsPath(newSmsPath);
    sms->setEnableNotifications(true);
    m_messages.insert(QDBusObjectPath(newSmsPath), sms);
    QDBusConnection::sessionBus().registerObject(newSmsPath, sms, QDBusConnection::ExportScriptableContents);
    Q_EMIT Added(QDBusObjectPath(newSmsPath), false);
}

void ModemMessaging::setDefaultStorage(uint defaultStorage)
{
    m_defaultStorage = defaultStorage;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("DefaultStorage"), m_defaultStorage);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Messaging") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemMessaging::setSupportedStorages(const ModemManager::UIntList &supportedStorages)
{
    m_supportedStorages = supportedStorages;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SupportedStorages"), QVariant::fromValue<ModemManager::UIntList>(m_supportedStorages));
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Messaging") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap ModemMessaging::toMap() const
{
    QVariantMap map;
#if MM_CHECK_VERSION(1, 2, 0)
    map.insert(QLatin1String(MM_MODEM_MESSAGING_PROPERTY_MESSAGES), QVariant::fromValue<QList<QDBusObjectPath>>(m_messages.keys()));
#endif
    map.insert(QLatin1String(MM_MODEM_MESSAGING_PROPERTY_SUPPORTEDSTORAGES), QVariant::fromValue<ModemManager::UIntList>(m_supportedStorages));
    map.insert(QLatin1String(MM_MODEM_MESSAGING_PROPERTY_DEFAULTSTORAGE), m_defaultStorage);
    return map;
}

#include "moc_modemmessaging.cpp"
