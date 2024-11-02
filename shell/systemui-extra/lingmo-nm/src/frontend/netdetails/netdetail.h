/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef NETDETAIL_H
#define NETDETAIL_H

#include <QDialog>
#include <QPainter>
#include <QObject>
#include <QStackedWidget>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariantMap>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QThread>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCloseEvent>

#include "detailpage.h"
#include "ipv4page.h"
#include "ipv6page.h"
#include "securitypage.h"
#include "creatnetpage.h"
#include "configpage.h"
#include "coninfo.h"
#include "tab-pages/tabpage.h"
#include "kwidget.h"
#include "ktabbar.h"
#include "networkmodeconfig.h"

#include <arpa/inet.h>
using namespace kdk;

#define  TAB_WIDTH  60
#define  TAB_HEIGHT 36
#define  TAB_HEIGHT_TABLET 48

class NetTabBar : public KTabBar
{
    Q_OBJECT
public:
    explicit NetTabBar(QWidget *parent = nullptr);
    ~NetTabBar();
protected:
    QSize sizeHint() const;
    QSize minimumTabSizeHint(int index) const;

};
class ThreadObject : public QObject
{
    Q_OBJECT
public:
    ThreadObject(QString deviceName, QObject *parent = nullptr);
    ~ThreadObject();
    void stop();
private:
    QString m_devName;
    volatile bool m_isStop;

public Q_SLOTS:
    void checkIpv4ConflictThread(const QString &ipv4Address);
    void checkIpv6ConflictThread(const QString &ipv6Address);

Q_SIGNALS:
    bool ipv4IsConflict(bool isConflict);
    bool ipv6IsConflict(bool isConflict);
};

class NetDetail : public QWidget
{
    Q_OBJECT

public:
    NetDetail(QString interface, QString name, QString uuid, bool isActive, bool isWlan, bool isCreateNet, QWidget *parent = nullptr);
    ~NetDetail();
    void centerToScreen();

    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *w, QEvent *event);

private:
    void initUI();
    void initComponent();
    void getConInfo(ConInfo &conInfo);
    void loadPage();
    void pagePadding(QString netName, bool isWlan);
    void initSecuData();

    void initTlsInfo(ConInfo &conInfo);
    void initPeapInfo(ConInfo &conInfo);
    void initTtlsInfo(ConInfo &conInfo);
    void initLeapInfo(ConInfo &conInfo);
    void initPwdInfo(ConInfo &conInfo);
    void initFastInfo(ConInfo &conInfo);

    void updateWirelessPersonalConnect();
    void updateWirelessEnterPriseConnect(KyEapMethodType enterpriseType);

    //详情ssid 带宽 物理地址 无线额外(安全性 频带 通道)
    void getBaseInfo(ConInfo &conInfo);
    //详情ipv4 ipv6 ipv4Dns
    void getDynamicIpInfo(ConInfo &conInfo, bool bActived);
    //ipv4+ipv6页面
    void getStaticIpInfo(ConInfo &conInfo, bool bActived);

    void setConfirmEnable();

//    bool checkIpv4Conflict(QString ipv4Address);
//    bool checkIpv6Conflict(QString ipv6Address);

    bool createWiredConnect();
    bool createWirelessConnect();
    bool updateConnect();

    bool checkWirelessSecurity(KySecuType secuType);

    void showDesktopNotify(const QString &message, QString soundName);

    void setNetdetailSomeEnable(bool on);

    void startObjectThread();
    void setNetTabToolTip();

    void getIpv4Ipv6Info(QString objPath, ConInfo &conInfo);
    QMap<QString, QVariant> getAddressDataFromMap(QMap<QString,QVariant> &innerMap, QString innerKey);
    KyIpConfigType getIpConfigTypeFromMap(QMap<QString,QVariant> &innerMap, QString innerKey);
    QList<QHostAddress> getIpv4DnsFromMap(QMap<QString,QVariant> &innerMap, QString innerKey);
    QList<QHostAddress> getIpv6DnsFromMap(QMap<QString,QVariant> &innerMap, QString innerKey);

private:
    KyNetworkDeviceResourse *m_netDeviceResource = nullptr;
    KyConnectOperation* m_connectOperation = nullptr;
    KyWirelessConnectOperation *m_wirelessConnOpration = nullptr;
    KyWiredConnectOperation *m_wiredConnOperation = nullptr;
    KyWirelessNetResource *m_resource = nullptr;

    QStackedWidget * stackWidget;

    DetailPage     * detailPage;
    Ipv4Page       * ipv4Page;
    Ipv6Page       * ipv6Page;
    SecurityPage   * securityPage;
    CreatNetPage   * createNetPage;
    ConfigPage     * configPage;

    QWidget      * centerWidget;
    QWidget      * bottomWidget;
    QScrollArea  * m_secuPageScrollArea;
    QScrollArea  * m_ipv4ScrollArea;
    QScrollArea  * m_ipv6ScrollArea;
    QScrollArea  * m_createNetPageScrollArea;

    QPushButton  * cancelBtn;
    QPushButton  * forgetBtn;
    QPushButton  * confimBtn;

    QFrame       * pageFrame;
    NetTabBar      *m_netTabBar = nullptr;

    QString      m_name;
    QString      m_uuid;
    QString      m_deviceName;

    bool         isWlan;
    bool         m_isCreateNet;
    bool         isActive;
    bool         isHideWlan;

    bool         isCreateOk;
    bool         isDetailOk;
    bool         isIpv4Ok;
    bool         isIpv6Ok;
    bool         isSecuOk;
    bool         isConfirmBtnEnable;

    ConInfo      m_info;

    ThreadObject *m_object;
    QThread *m_objectThread;
    NetworkModeType m_networkMode = DBUS_INVAILD;

private Q_SLOTS:
    void on_btnConfirm_clicked();
    void on_btnForget_clicked();
    void onPaletteChanged();

protected Q_SLOTS:
    void currentRowChangeSlot(int row);

Q_SIGNALS:
    void detailPageClose(QString, QString, QString);
    void createPageClose(QString);
    void currentChanged(int);
    void checkCurrentIpv4Conflict(const QString &address);
    void checkCurrentIpv6Conflict(const QString &address);
};
#endif // NETDETAIL_H
