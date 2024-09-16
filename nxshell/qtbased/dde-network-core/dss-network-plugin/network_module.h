// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include "tray_module_interface.h"
#include "../common-plugin/utils.h"
#include "item/devicestatushandler.h"

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WiredDevice>

class QLabel;
NETWORKPLUGIN_BEGIN_NAMESPACE
class NetworkPluginHelper;
class NetworkDialog;
class SecretAgent;
NETWORKPLUGIN_END_NAMESPACE

namespace dss {
namespace module {
class PopupAppletManager;
class NetworkPanelContainer : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkPanelContainer(NETWORKPLUGIN_NAMESPACE::NetworkDialog *dialog, QWidget *parent = nullptr);
    ~NetworkPanelContainer() override;
    void sendWarnMessage(const QString &msg);
    void clearWarnMessage();
    void setContentWidget(QWidget *content);
    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
    void onPluginStateChanged(PluginState state);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QLabel *m_warnLabel;
    NETWORKPLUGIN_NAMESPACE::NetworkDialog *m_dialog;
    QWidget *m_savedParent;
    QWidget *m_contentWidget;
};
/**
 * @brief The NetworkModule class
 * 用于处理插件差异
 * NetworkModule处理信号槽有问题，固增加该类
 */
class NetworkModule : public QObject
{
    Q_OBJECT

public:
    explicit NetworkModule(QObject *parent = nullptr);
    ~NetworkModule() override;

    QWidget *content();
    QWidget *itemTipsWidget() const;
    const QString itemContextMenu() const;
    void invokedMenuItem(const QString &menuId, const bool checked) const;

    NETWORKPLUGIN_NAMESPACE::NetworkPluginHelper *networkHelper() const { return m_networkHelper; }

protected Q_SLOTS:
    void updateLockScreenStatus(bool visible);
    void onDeviceStatusChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);
    void onAddDevice(const QString &path);
    void onUserChanged(QString json);

private:
    void addFirstConnection(NetworkManager::WiredDevice *nmDevice);
    bool hasConnection(NetworkManager::WiredDevice *nmDevice, NetworkManager::Connection::List &unSaveDevices);
    const QString connectionMatchName() const;
    void installTranslator(QString locale);
    bool needPopupNetworkDialog() const;

private:
    NETWORKPLUGIN_NAMESPACE::NetworkPluginHelper *m_networkHelper;
    NETWORKPLUGIN_NAMESPACE::NetworkDialog *m_networkDialog;
    NETWORKPLUGIN_NAMESPACE::SecretAgent *m_secretAgent;
    QPointer<NetworkPanelContainer> m_panelContainer;

    bool m_isLockModel;  // 锁屏 or greeter
    bool m_isLockScreen; // 锁屏显示
    bool m_initilized;

    QSet<QString> m_devicePaths; // 记录无线设备Path,防止信号重复连接
    QString m_lastActiveWirelessDevicePath;
    QString m_lastConnection;
    QString m_lastConnectionUuid;
    NetworkManager::Device::State m_lastState;
    int m_clickTime;
};

class NetworkPlugin : public QObject, public TrayModuleInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "com.deepin.dde.shell.Modules.Tray" FILE "network.json")
    Q_INTERFACES(dss::module::TrayModuleInterface)

public:
    explicit NetworkPlugin(QObject *parent = nullptr);
    ~NetworkPlugin() override { }
    void init() override;
    bool isNeedInitPlugin() const { return true; }

    inline QString key() const override { return objectName(); }
    QWidget *content() override;
    QString icon() const override;

    QWidget *itemWidget() const override;
    QWidget *itemTipsWidget() const override;
    const QString itemContextMenu() const override;
    void invokedMenuItem(const QString &menuId, const bool checked) const override;

private:
    void ensureNetwork();

private:
    NetworkModule *m_network;
};

} // namespace module
} // namespace dss
#endif // LOGIN_MODULE_H
