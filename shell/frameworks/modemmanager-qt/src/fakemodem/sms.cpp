/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "sms.h"

#include <QDBusConnection>
#include <QDBusMessage>

Sms::Sms(QObject *parent)
    : QObject(parent)
    , m_enabledNotifications(false)
    , m_smsClass(0)
    , m_deliveryReportRequest(false)
    , m_deliveryState(0)
    , m_messageReference(0)
    , m_pduType(0)
    , m_serviceCategory(0)
    , m_state(0)
    , m_storage(0)
    , m_teleserviceId(0)
    , m_validity({MM_SMS_VALIDITY_TYPE_RELATIVE, MM_SMS_VALIDITY_TYPE_ENHANCED})
{
}

Sms::~Sms()
{
}

int Sms::smsClass() const
{
    return m_smsClass;
}

QByteArray Sms::data() const
{
    return m_data;
}

bool Sms::deliveryReportRequest() const
{
    return m_deliveryReportRequest;
}

uint Sms::deliveryState() const
{
    return m_deliveryState;
}

QString Sms::dischargeTimestamp() const
{
    return m_dischargeTimestamp;
}

uint Sms::messageReference() const
{
    return m_messageReference;
}

QString Sms::number() const
{
    return m_number;
}

uint Sms::pduType() const
{
    return m_pduType;
}

QString Sms::SMSC() const
{
    return m_SMSC;
}

uint Sms::serviceCategory() const
{
    return m_serviceCategory;
}

uint Sms::state() const
{
    return m_state;
}

uint Sms::storage() const
{
    return m_storage;
}

uint Sms::teleserviceId() const
{
    return m_teleserviceId;
}

QString Sms::text() const
{
    return m_text;
}

QString Sms::timestamp() const
{
    return m_timestamp;
}

ModemManager::ValidityPair Sms::validity() const
{
    return m_validity;
}

void Sms::Send()
{
}

void Sms::Store(uint storage)
{
    Q_UNUSED(storage);
}

QString Sms::smsPath() const
{
    return m_smsPath;
}

void Sms::setSmsPath(const QString &path)
{
    m_smsPath = path;
}

void Sms::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void Sms::setSmsClass(int smsClass)
{
    m_smsClass = smsClass;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Class"), m_smsClass);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setData(const QByteArray &data)
{
    m_data = data;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Data"), m_data);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setDeliveryReportRequest(bool deliveryReportRequest)
{
    m_deliveryReportRequest = deliveryReportRequest;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("DeliveryReportRequest"), m_deliveryReportRequest);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setDeliveryState(uint state)
{
    m_deliveryState = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("DeliveryState"), m_deliveryState);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setDischargeTimestamp(const QString &timestamp)
{
    m_dischargeTimestamp = timestamp;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("DischargeTimestamp"), m_dischargeTimestamp);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setMessageReference(uint messageReference)
{
    m_messageReference = messageReference;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("MessageReference"), m_messageReference);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setNumber(const QString &number)
{
    m_number = number;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Number"), m_number);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}
void Sms::setPduType(uint pduType)
{
    m_pduType = pduType;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("PduType"), m_pduType);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setSMSC(const QString &smsc)
{
    m_SMSC = smsc;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SMSC"), m_SMSC);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setServiceCategory(uint serviceCategory)
{
    m_serviceCategory = serviceCategory;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("ServiceCategory"), m_serviceCategory);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setState(uint state)
{
    m_state = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("State"), m_state);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setStorage(uint storage)
{
    m_storage = storage;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Storage"), m_storage);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setTeleserviceId(uint teleserviceId)
{
    m_teleserviceId = teleserviceId;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("TeleserviceId"), m_teleserviceId);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setText(const QString &text)
{
    m_text = text;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Text"), m_text);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setTimestamp(const QString &timestamp)
{
    m_timestamp = timestamp;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Timestamp"), m_timestamp);
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Sms::setValidity(const ModemManager::ValidityPair &validity)
{
    m_validity.validity = validity.validity;
    m_validity.value = validity.value;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Validity"), QVariant::fromValue<ModemManager::ValidityPair>(m_validity));
        QDBusMessage message = QDBusMessage::createSignal(m_smsPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Sms") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

#include "moc_sms.cpp"
