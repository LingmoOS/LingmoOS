/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem.h"

#include <QDBusConnection>
#include <QDBusMessage>

Modem::Modem(QObject *parent)
    : QObject(parent)
    , m_accessTechnologies(0)
    , m_currentCapabilities(0)
    , m_enabledNotifications(false)
    , m_maxActiveBearers(0)
    , m_maxBearers(0)
    , m_powerState(0)
    , m_state(0)
    , m_stateFailedReason(0)
    , m_supportedBands{0}
    , m_supportedIpFamilies(0)
    , m_unlockRequired(0)
{
}

Modem::~Modem()
{
}

uint Modem::accessTechnologies() const
{
    return m_accessTechnologies;
}

QList<QDBusObjectPath> Modem::bearers() const
{
    return m_bearers;
}

ModemManager::UIntList Modem::currentBands() const
{
    return m_currentBands;
}

uint Modem::currentCapabilities() const
{
    return m_currentCapabilities;
}

ModemManager::CurrentModesType Modem::currentModes() const
{
    return m_currentModes;
}

QString Modem::device() const
{
    return m_device;
}

QString Modem::deviceIdentifier() const
{
    return m_deviceIdentifier;
}

QStringList Modem::drivers() const
{
    return m_drivers;
}

QString Modem::equipmentIdentifier() const
{
    return m_equipmentIdentifier;
}

QString Modem::manufacturer() const
{
    return m_manufacturer;
}

uint Modem::maxActiveBearers() const
{
    return m_maxActiveBearers;
}

uint Modem::maxBearers() const
{
    return m_maxBearers;
}

QString Modem::modemPath() const
{
    return m_path;
}

QString Modem::model() const
{
    return m_model;
}

QStringList Modem::ownNumbers() const
{
    return m_ownNumbers;
}

QString Modem::plugin() const
{
    return m_plugin;
}

ModemManager::PortList Modem::ports() const
{
    return m_ports;
}

uint Modem::powerState() const
{
    return m_powerState;
}

QString Modem::primaryPort() const
{
    return m_primaryPort;
}

QString Modem::revision() const
{
    return m_revision;
}

ModemManager::SignalQualityPair Modem::signalQuality() const
{
    return m_signalQuality;
}

QDBusObjectPath Modem::sim() const
{
    return m_sim;
}

int Modem::state() const
{
    return m_state;
}

uint Modem::stateFailedReason() const
{
    return m_stateFailedReason;
}

ModemManager::UIntList Modem::supportedBands() const
{
    return m_supportedBands;
}

ModemManager::UIntList Modem::supportedCapabilities() const
{
    return m_supportedCapabilities;
}

uint Modem::supportedIpFamilies() const
{
    return m_supportedIpFamilies;
}

ModemManager::SupportedModesType Modem::supportedModes() const
{
    return m_supportedModes;
}

uint Modem::unlockRequired() const
{
    return m_unlockRequired;
}

ModemManager::UnlockRetriesMap Modem::unlockRetries() const
{
    return m_unlockRetries;
}

void Modem::addBearer(const QDBusObjectPath &bearer)
{
    m_bearers << bearer;
}

void Modem::removeBearer(const QDBusObjectPath &bearer)
{
    m_bearers.removeAll(bearer);
}

void Modem::setDevice(const QString &device)
{
    m_device = device;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Device"), m_device);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void Modem::setDeviceIdentifier(const QString &deviceIdentifier)
{
    m_deviceIdentifier = deviceIdentifier;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("DeviceIdentifier"), m_deviceIdentifier);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setDrivers(const QStringList &drivers)
{
    m_drivers = drivers;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Drivers"), m_deviceIdentifier);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setEquipmentIdentifier(const QString &identifier)
{
    m_equipmentIdentifier = identifier;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("EquipmentIdentifier"), m_equipmentIdentifier);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Manufacturer"), m_manufacturer);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setMaxActiveBearers(uint bearers)
{
    m_maxActiveBearers = bearers;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("MaxActiveBearers"), m_maxActiveBearers);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setMaxBearers(uint bearers)
{
    m_maxBearers = bearers;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("MaxBearers"), m_maxBearers);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setModemPath(const QString &path)
{
    m_path = path;
}

void Modem::setModel(const QString &model)
{
    m_model = model;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Model"), m_model);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setOwnNumbers(const QStringList &numbers)
{
    m_ownNumbers = numbers;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("OwnNumbers"), m_ownNumbers);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setPlugin(const QString &plugin)
{
    m_plugin = plugin;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Plugin"), m_plugin);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setPorts(const ModemManager::PortList &ports)
{
    m_ports = ports;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Ports"), QVariant::fromValue<ModemManager::PortList>(m_ports));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setPrimaryPort(const QString &port)
{
    m_primaryPort = port;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("PrimaryPort"), m_primaryPort);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setRevision(const QString &revision)
{
    m_revision = revision;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Revision"), m_revision);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSignalQuality(const ModemManager::SignalQualityPair &signalQuality)
{
    m_signalQuality = signalQuality;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SignalQuality"), QVariant::fromValue<ModemManager::SignalQualityPair>(m_signalQuality));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSim(const QDBusObjectPath &sim)
{
    m_sim = sim;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Sim"), QVariant::fromValue<QDBusObjectPath>(m_sim));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setState(int state)
{
    int previousState = m_state;
    m_state = state;

    if (m_enabledNotifications) {
        Q_EMIT StateChanged(previousState, m_state, 0);
    }
}

void Modem::setStateFailedReason(uint reason)
{
    m_stateFailedReason = reason;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("StateFailedReason"), m_stateFailedReason);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSupportedBands(const ModemManager::UIntList &bands)
{
    m_supportedBands = bands;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SupportedBands"), QVariant::fromValue<ModemManager::UIntList>(m_supportedBands));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSupportedCapabilities(const ModemManager::UIntList &capabilities)
{
    m_supportedCapabilities = capabilities;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SupportedCapabilities"), QVariant::fromValue<ModemManager::UIntList>(m_supportedCapabilities));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSupportedIpFamilies(uint families)
{
    m_supportedIpFamilies = families;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SupportedIpFamilies"), m_supportedIpFamilies);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setSupportedModes(const ModemManager::SupportedModesType &modes)
{
    m_supportedModes = modes;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SupportedModes"), QVariant::fromValue<ModemManager::SupportedModesType>(m_supportedModes));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setUnlockRequired(uint unlockRequired)
{
    m_unlockRequired = unlockRequired;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("UnlockRequired"), m_unlockRequired);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setUnlockRetries(const ModemManager::UnlockRetriesMap &unlockRetries)
{
    m_unlockRetries = unlockRetries;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("UnlockRetries"), QVariant::fromValue<ModemManager::UnlockRetriesMap>(m_unlockRetries));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::setAccessTechnologies(uint technologies)
{
    m_accessTechnologies = technologies;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("AccessTechnologies"), m_accessTechnologies);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::SetCurrentBands(const QList<uint> &bands)
{
    m_currentBands = bands;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("CurrentBands"), QVariant::fromValue<QList<uint>>(m_currentBands));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::SetCurrentCapabilities(uint capabilities)
{
    m_currentCapabilities = capabilities;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("CurrentCapabilities"), m_currentCapabilities);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::SetCurrentModes(ModemManager::CurrentModesType modes)
{
    m_currentModes = modes;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("CurrentModes"), QVariant::fromValue<ModemManager::CurrentModesType>(m_currentModes));
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem::SetPowerState(uint state)
{
    m_powerState = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("PowerState"), m_powerState);
        QDBusMessage message = QDBusMessage::createSignal(m_path, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QString Modem::Command(const QString &cmd, uint timeout)
{
    Q_UNUSED(cmd);
    Q_UNUSED(timeout);
    // TODO
    return QString();
}

QDBusObjectPath Modem::CreateBearer(const QVariantMap &properties)
{
    Q_UNUSED(properties);
    // TODO
    return QDBusObjectPath();
}

void Modem::DeleteBearer(const QDBusObjectPath &bearer)
{
    Q_UNUSED(bearer);
    // TODO
}

void Modem::Enable(bool enable)
{
    Q_UNUSED(enable);
    // TODO
}

void Modem::FactoryReset(const QString &code)
{
    Q_UNUSED(code);
    // TODO
}

QList<QDBusObjectPath> Modem::ListBearers()
{
    return m_bearers;
}

void Modem::Reset()
{
    // TODO
}

QVariantMap Modem::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SIM), QVariant::fromValue<QDBusObjectPath>(m_sim));
#if MM_CHECK_VERSION(1, 2, 0)
    map.insert(QLatin1String(MM_MODEM_PROPERTY_BEARERS), QVariant::fromValue<QList<QDBusObjectPath>>(m_bearers));
#endif
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDCAPABILITIES), QVariant::fromValue<ModemManager::UIntList>(m_supportedCapabilities));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_CURRENTCAPABILITIES), m_currentCapabilities);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_MAXBEARERS), m_maxBearers);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_MAXACTIVEBEARERS), m_maxActiveBearers);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_MANUFACTURER), m_manufacturer);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_MODEL), m_model);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_REVISION), m_revision);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_DEVICEIDENTIFIER), m_deviceIdentifier);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_DEVICE), m_device);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_DRIVERS), m_drivers);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_PLUGIN), m_plugin);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_PRIMARYPORT), m_primaryPort);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_PORTS), QVariant::fromValue<ModemManager::PortList>(m_ports));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_EQUIPMENTIDENTIFIER), m_equipmentIdentifier);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_UNLOCKREQUIRED), m_unlockRequired);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_UNLOCKRETRIES), QVariant::fromValue<ModemManager::UnlockRetriesMap>(m_unlockRetries));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_STATE), m_state);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_STATEFAILEDREASON), m_stateFailedReason);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_ACCESSTECHNOLOGIES), m_accessTechnologies);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SIGNALQUALITY), QVariant::fromValue<ModemManager::SignalQualityPair>(m_signalQuality));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_OWNNUMBERS), m_ownNumbers);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_POWERSTATE), m_powerState);
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDMODES), QVariant::fromValue<ModemManager::SupportedModesType>(m_supportedModes));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_CURRENTMODES), QVariant::fromValue<ModemManager::CurrentModesType>(m_currentModes));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDBANDS), QVariant::fromValue<ModemManager::UIntList>(m_supportedBands));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_CURRENTBANDS), QVariant::fromValue<ModemManager::UIntList>(m_currentBands));
    map.insert(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDIPFAMILIES), m_supportedIpFamilies);
    return map;
}

#include "moc_modem.cpp"
