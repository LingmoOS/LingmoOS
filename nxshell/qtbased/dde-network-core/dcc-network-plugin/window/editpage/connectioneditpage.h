// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONNECTIONEDITPAGE_H
#define CONNECTIONEDITPAGE_H

#include "settings/abstractsettings.h"
#include "parameterscontainer.h"

#include "interface/namespace.h"

#include <QPointer>

#include <DAbstractDialog>

#include <networkmanagerqt/connection.h>
#include <networkmanagerqt/connectionsettings.h>

namespace DCC_NAMESPACE {
class ButtonTuple;
}

class QPushButton;
class QEvent;

class QVBoxLayout;

class ConnectionEditPage : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT

public:
    // Keep the same enum value as ConnectionSettings::ConnectionType
    enum ConnectionType {
        PppoeConnection = 9,
        VpnConnection = 11,
        WiredConnection = 13,
        WirelessConnection = 14
    };

    // Connection will NOT be activated if "devPath" is empty
    // A NEW connection will be created if "connUuid" is empty
    explicit ConnectionEditPage(ConnectionType connType, const QString &devPath, const QString &connUuid = QString(), QWidget *parent = nullptr, bool isHotSpot = false);
    virtual ~ConnectionEditPage() Q_DECL_OVERRIDE;

    // This method or some similar method of Subclass must be called after initialization
    virtual void initSettingsWidget();

    static const QString devicePath();
    void setDevicePath(const QString &path);
    inline const QString connectionUuid() { return m_connectionUuid; }

    void onDeviceRemoved();

    void setButtonTupleEnable(bool enable);
    void setLeftButtonEnable(bool enable);

Q_SIGNALS:
    void requestNextPage(QWidget *const page);
    void requestFrameAutoHide(const bool autoHide) const;
    void requestWiredDeviceEnabled(const QString &devPath, const bool enabled) const;
    void activateWiredConnection(const QString &connString, const QString &uuid);
    void activateWirelessConnection(const QString &ssid, const QString &uuid);
    void activateVpnConnection(const QString &, const QString &);
    // private signals
    void saveSettingsDone();
    void prepareConnectionDone();
    void disconnect(const QString &);

protected:
    int connectionSuffixNum(const QString &matchConnName);
    void addHeaderButton(QPushButton *button);
    void prepareConnection();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initUI();
    void initHeaderButtons();
    void initConnection();
    void initConnectionSecrets();
    void saveConnSettings();
    void updateConnection();
    void createConnSettings();
    bool isConnected();

    NMVariantMapMap secretsMapMapBySettingType(NetworkManager::Setting::SettingType settingType);
    // T means a Setting subclass, like WirelessSecuritySetting
    template<typename T>
    void setSecretsFromMapMap(NetworkManager::Setting::SettingType settingType, NMVariantMapMap secretsMapMap);

protected Q_SLOTS:
    void onRequestNextPage(QWidget *const page);
    void onRemoveButton();

protected:
    QVBoxLayout *m_settingsLayout;

    NetworkManager::Connection::Ptr m_connection;
   NetworkManager:: ConnectionSettings::Ptr m_connectionSettings;

    AbstractSettings *m_settingsWidget;

    ParametersContainer::Ptr m_tempParameter;
    bool m_isNewConnection;
    QString m_connectionUuid;

private:
    QVBoxLayout *m_mainLayout;
    QPushButton *m_disconnectBtn;
    QPushButton *m_removeBtn;
    DCC_NAMESPACE::ButtonTuple *m_buttonTuple;
    DCC_NAMESPACE::ButtonTuple *m_buttonTuple_conn;

    QPointer<QWidget> m_subPage;

    NetworkManager::ConnectionSettings::ConnectionType m_connType;

    bool m_isHotSpot;
};

#endif /* CONNECTIONEDITPAGE_H */
