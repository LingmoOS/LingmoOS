#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WiredSetting>
#include <QDebug>

#include "wiressitemsettings.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

WirelessItemSettings::WirelessItemSettings(QObject *parent)
    : QObject(parent)
{
}

QString WirelessItemSettings::path() const
{
    return m_path;
}

void WirelessItemSettings::setPath(const QString &path)
{
    if (m_path == path)
        return;

    m_path = path;

    m_connection = NetworkManager::findConnection(m_path);
    if (m_connection) {
        m_settings = m_connection->settings();
        Q_EMIT nameChanged();
        Q_EMIT autoConnectChanged();
        Q_EMIT availableToOtherUsersChanged();
        Q_EMIT settingsAvailable();
    } else {
        m_settings.reset();
    }

    Q_EMIT pathChanged();
}

QString WirelessItemSettings::name() const
{
    if (m_settings)
        return m_settings->id();
    else
        qWarning("NetworkManager settings are invalid");

    return QString();
}

void WirelessItemSettings::setName(const QString &name)
{
    if (m_settings) {
        if (this->name() == name)
            return;
        m_settings->setId(name);
        m_connection->update(m_settings->toMap());
        Q_EMIT nameChanged();
    } else {
        qWarning("NetworkManager settings are invalid");
    }
}

bool WirelessItemSettings::autoConnect() const
{
    if (m_settings)
        return m_settings->autoconnect();
    else
        qWarning("NetworkManager settings are invalid");
    return false;
}

void WirelessItemSettings::setAutoConnect(bool value)
{
    if (m_settings) {
        if (this->autoConnect() == value)
            return;
        m_settings->setAutoconnect(value);
        m_connection->update(m_settings->toMap());
        Q_EMIT autoConnectChanged();
    } else {
        qWarning("NetworkManager settings are invalid");
    }
}

bool WirelessItemSettings::availableToOtherUsers() const
{
    if (m_settings)
        return m_settings->permissions().isEmpty();
    else
        qWarning("NetworkManager settings are invalid");
    return false;
}

void WirelessItemSettings::setAvailableToOtherUsers(bool value)
{
    if (m_settings) {
        bool availableToOtherUsers = m_settings->permissions().isEmpty();
        if (availableToOtherUsers == value)
            return;

        if (value)
            m_settings->setPermissions(QHash<QString, QString>());
        else
            m_settings->addToPermissions(getCurrentUserName(), QString());
        m_connection->update(m_settings->toMap());
        Q_EMIT availableToOtherUsersChanged();
    } else {
        qWarning("NetworkManager settings are invalid");
    }
}

void WirelessItemSettings::updateSettings(const QString &type, const QVariantMap &map)
{
    if (!m_settings)
        return;

    NMVariantMapMap settings = m_settings->toMap();
    settings.insert(type, map);
    m_connection->update(settings);
}

QString WirelessItemSettings::getCurrentUserName() const
{
    uid_t uid = ::geteuid();
    struct passwd *pw = ::getpwuid(uid);
    if (pw)
        return QString::fromLocal8Bit(pw->pw_name);
    return QString();
}
