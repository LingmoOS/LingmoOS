#ifndef WIRELESSITEMSETTINGS_H
#define WIRELESSITEMSETTINGS_H

#include <networkmanager_export.h>

#include <QtCore/QObject>

#include <NetworkManagerQt/Connection>

class NETWORKMANAGER_EXPORT WirelessItemSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)
    Q_PROPERTY(bool availableToOtherUsers READ availableToOtherUsers WRITE setAvailableToOtherUsers NOTIFY availableToOtherUsersChanged)

public:
    WirelessItemSettings(QObject *parent = nullptr);

    QString path() const;
    void setPath(const QString &path);

    QString name() const;
    void setName(const QString &name);

    bool autoConnect() const;
    void setAutoConnect(bool value);

    bool availableToOtherUsers() const;
    void setAvailableToOtherUsers(bool value);

Q_SIGNALS:
    void pathChanged();
    void settingsAvailable();
    void nameChanged();
    void autoConnectChanged();
    void availableToOtherUsersChanged();

protected:
    NetworkManager::Connection::Ptr m_connection;
    NetworkManager::ConnectionSettings::Ptr m_settings;

    void updateSettings(const QString &type, const QVariantMap &map);

private:
    QString m_path;

    QString getCurrentUserName() const;
};

#endif // WIRELESSITEMSETTINGS_H
