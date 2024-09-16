// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationutil.h"
#include "cooperationutil_p.h"
#include "gui/mainwindow.h"

#include "configs/settings/configmanager.h"
#include "configs/dconfig/dconfigmanager.h"
#include "common/constant.h"
#include "common/commonutils.h"
#include "net/networkutil.h"

#include <QJsonDocument>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QTimer>

#include <mutex>

#ifdef linux
#    include <DFeatureDisplayDialog>
DWIDGET_USE_NAMESPACE
#endif
using namespace cooperation_core;

CooperationUtilPrivate::CooperationUtilPrivate(CooperationUtil *qq)
    : q(qq)
{
}

CooperationUtilPrivate::~CooperationUtilPrivate()
{
}

CooperationUtil::CooperationUtil(QObject *parent)
    : QObject(parent),
      d(new CooperationUtilPrivate(this))
{
}

CooperationUtil::~CooperationUtil()
{
}

CooperationUtil *CooperationUtil::instance()
{
    static CooperationUtil ins;
    return &ins;
}

void CooperationUtil::mainWindow(QSharedPointer<MainWindow> window)
{
    d->window = window;
}

void CooperationUtil::activateWindow()
{
    if (d->window) {
        d->window->activateWindow();
    }
}

void CooperationUtil::registerDeviceOperation(const QVariantMap &map)
{
    d->window->onRegistOperations(map);
}

void CooperationUtil::setStorageConfig(const QString &value)
{
    emit storageConfig(value);
}

QVariantMap CooperationUtil::deviceInfo()
{
    QVariantMap info;
#ifdef linux
    auto value = DConfigManager::instance()->value(kDefaultCfgPath, DConfigKey::DiscoveryModeKey, 0);
    int mode = value.toInt();
    mode = (mode < 0) ? 0 : (mode > 1) ? 1 : mode;
    info.insert(AppSettings::DiscoveryModeKey, mode);
#else
    auto value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::DiscoveryModeKey);
    info.insert(AppSettings::DiscoveryModeKey, value.isValid() ? value.toInt() : 0);
#endif

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::DeviceNameKey);
    info.insert(AppSettings::DeviceNameKey,
                value.isValid()
                        ? value.toString()
                        : QDir(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0)).dirName());

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::PeripheralShareKey);
    info.insert(AppSettings::PeripheralShareKey, value.isValid() ? value.toBool() : true);

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::LinkDirectionKey);
    info.insert(AppSettings::LinkDirectionKey, value.isValid() ? value.toInt() : 0);

#ifdef linux
    value = DConfigManager::instance()->value(kDefaultCfgPath, DConfigKey::TransferModeKey, 0);
    mode = value.toInt();
    mode = (mode < 0) ? 0 : (mode > 2) ? 2 : mode;
    info.insert(AppSettings::TransferModeKey, mode);
#else
    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::TransferModeKey);
    info.insert(AppSettings::TransferModeKey, value.isValid() ? value.toInt() : 0);
#endif

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::StoragePathKey);
    auto storagePath = value.isValid() ? value.toString() : QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    info.insert(AppSettings::StoragePathKey, storagePath);
    static std::once_flag flag;
    std::call_once(flag, [&storagePath] { CooperationUtil::instance()->setStorageConfig(storagePath); });

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::ClipboardShareKey);
    info.insert(AppSettings::ClipboardShareKey, value.isValid() ? value.toBool() : true);

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::CooperationEnabled);
    info.insert(AppSettings::CooperationEnabled, true);

    value = deepin_cross::BaseUtils::osType();
    info.insert(AppSettings::OSType, value);

    info.insert(AppSettings::IPAddress, localIPAddress());

    return info;
}

QString CooperationUtil::localIPAddress()
{
    QString ip = deepin_cross::CommonUitls::getFirstIp().data();
    return ip;
}

QString CooperationUtil::closeOption()
{
    QString option = ConfigManager::instance()->appAttribute(AppSettings::CacheGroup, AppSettings::CloseOptionKey).toString();
    return option;
}

void CooperationUtil::saveOption(bool exit)
{
    ConfigManager::instance()->setAppAttribute(AppSettings::CacheGroup, AppSettings::CloseOptionKey,
                                               exit ? "Exit" : "Minimise");
}

void CooperationUtil::initNetworkListener()
{
    QTimer *networkMonitorTimer = new QTimer(this);
    networkMonitorTimer->setInterval(1000);
    connect(networkMonitorTimer, &QTimer::timeout, this, &CooperationUtil::checkNetworkState);
    networkMonitorTimer->start();
    emit onlineStateChanged(deepin_cross::CommonUitls::getFirstIp().c_str());
}

void CooperationUtil::checkNetworkState()
{
    // 网络状态检测
    bool isConnected = deepin_cross::CommonUitls::getFirstIp().size() > 0;

    if (isConnected != d->isOnline) {
        d->isOnline = isConnected;
        Q_EMIT onlineStateChanged(deepin_cross::CommonUitls::getFirstIp().c_str());
    }
}

void CooperationUtil::showFeatureDisplayDialog(QDialog *dlg1)
{
#ifdef linux
    DFeatureDisplayDialog *dlg = static_cast<DFeatureDisplayDialog *>(dlg1);
    auto btn = dlg->getButton(0);
    btn->setText(tr("View Help Manual"));
    dlg->setTitle(tr("Welcome to dde-cooperation"));
    dlg->addItem(new DFeatureItem(QIcon::fromTheme(":/icons/deepin/builtin/icons/tip_kvm.png"),
                                  tr("Keyboard and mouse sharing"), tr("When a connection is made between two devices, the initiator's keyboard and mouse can be used to control the other device"), dlg));
    dlg->addItem(new DFeatureItem(QIcon::fromTheme(":/icons/deepin/builtin/icons/tip_clipboard.png"),
                                  tr("Clipboard sharing"), tr("Once a connection is made between two devices, the clipboard will be shared and can be copied on one device and pasted on the other device"), dlg));
    dlg->addItem(new DFeatureItem(QIcon::fromTheme(":/icons/deepin/builtin/icons/tip_file.png"),
                                  tr("Delivery of documents"), tr("After connecting between two devices, you can initiate a file delivery to the other device"), dlg));
    dlg->addItem(new DFeatureItem(QIcon::fromTheme(":/icons/deepin/builtin/icons/tip_more.png"),
                                  tr("Usage"), tr("For detailed instructions, please click on the Help Manual below"), dlg));
    dlg->show();
#endif
}
