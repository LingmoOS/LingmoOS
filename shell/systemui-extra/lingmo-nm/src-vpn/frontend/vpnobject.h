/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef VPNOBJECT_H
#define VPNOBJECT_H

#include <QObject>
#include <QWidget>
#include <QScreen>
#include <QSystemTrayIcon>
#include <QMainWindow>

#include "vpnpage.h"
#include "vpnaddpage.h"
class VpnPage;

#define VISIBLE "visible"
#define GSETTINGS_VPNICON_VISIBLE "org.lingmo.lingmo-nm.vpnicon"
//const QByteArray GSETTINGS_VPNICON_VISIBLE = "org.lingmo.lingmo-nm.vpnicon";


class vpnObject : public QMainWindow
{
    Q_OBJECT
public:
    explicit vpnObject(QMainWindow *parent = nullptr);
    ~vpnObject();

    void getVirtualList(QVector<QStringList> &vector);
    //Vpn连接删除
    void deleteVpn(const QString &connUuid);
    //有线连接断开
    void activateVpn(const QString& connUuid);
    void deactivateVpn(const QString& connUuid);

    void showDetailPage(const QString& connUuid);

    void showVpnAddWidget();

private:
    void initUI();
    void initTrayIcon();
    void initVpnIconVisible();
    void initDbusConnnect();

    bool launchApp(QString desktopFile);
    void runExternalApp();

private:
    VpnPage *m_vpnPage = nullptr;
    QSystemTrayIcon * m_vpnTrayIcon = nullptr;
    QGSettings * m_vpnGsettings;   //VPN配置文件
    double tran =1;
    QGSettings       *StyleSettings = nullptr;
    QWidget * vpnWidget = nullptr;

    QDBusInterface * m_positionInterface = nullptr;
    bool m_isShowInCenter = false;

    vpnAddPage *m_vpnAddPage = nullptr;

public Q_SLOTS:
    void onShowMainWindow();

private Q_SLOTS:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onTabletModeChanged(bool mode);


Q_SIGNALS:
    void vpnAdd(QStringList info);
    void vpnRemove(QString dbusPath);
    void vpnUpdate(QStringList info);
    void vpnActiveConnectionStateChanged(QString uuid, int status);
    void activateFailed(QString errorMessage);
    void deactivateFailed(QString errorMessage);
    void mainWindowVisibleChanged(const bool &visible);
};

#endif // VPNOBJECT_H
