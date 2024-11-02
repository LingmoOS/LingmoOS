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
#ifndef VPNDETAIL_H
#define VPNDETAIL_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "kwidget.h"
#include "ktabbar.h"
#include "../tools/divider.h"
#include "vpnconfigpage.h"
#include "vpnipv4page.h"
#include "vpnipv6page.h"
#include "vpnadvancedpage.h"

#include "kyvpnconnectoperation.h"


using namespace kdk;

class VpnTabBar : public KTabBar
{
    Q_OBJECT
public:
    explicit VpnTabBar(QWidget *parent = nullptr);
    ~VpnTabBar() = default;

protected:
    QSize sizeHint() const;
    QSize minimumTabSizeHint(int index) const;

private Q_SLOTS:
    void onModeChanged(bool mode);

};

class VpnDetail : public QWidget
{
    Q_OBJECT
public:
    explicit VpnDetail(QString vpnUuid, QString vpnName, QWidget *parent = nullptr);
    void centerToScreen();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    void initWindow();
    void initTabBar();

    void initConnection();
    void setConfirmEnable();

    void getVpnConfig(QString connectUuid, KyVpnConfig &vpnInfo);
    void pagePadding(bool isOpenVpn);

    bool updateVpnCnofig();
    void setVpndetailSomeEnable(bool on);

    bool m_isOpenVpn = false;
    QString m_vpnName;
    QString m_uuid;
    KyVpnConfig m_vpnInfo;

    QStackedWidget * m_stackWidget;
    QWidget  *m_topWidget;
    QWidget  *m_centerWidget;
    Divider  *m_divider = nullptr;
    QWidget  *m_bottomWidget;

    VpnConfigPage   *m_configPage = nullptr;
    VpnIpv4Page     *m_ipv4Page = nullptr;
    VpnIpv6Page     *m_ipv6Page = nullptr;
    VpnAdvancedPage *m_advancedPage = nullptr;

    QScrollArea *m_configScroArea =  nullptr;
    QScrollArea *m_ipv4ScroArea =  nullptr;
    QScrollArea *m_ipv6ScroArea =  nullptr;
    QScrollArea *m_advancedScroArea =  nullptr;
    VpnTabBar   *m_vpnTabBar = nullptr;
    QCheckBox   *m_autoConnectBox = nullptr;
    QLabel      *m_autoConnectLabel;
    QPushButton *m_cancelBtn;
    QPushButton *m_confimBtn;

    bool m_isConfigOk = false;
    bool m_isIpv4Ok = false;
    bool m_isIpv6Ok = false;
    bool m_isAdvancedOk = false;

protected Q_SLOTS:
    void onTabCurrentRowChange(int row);
    void onConfimBtnClicked();

Q_SIGNALS:
    void currentChanged(int);

};

#endif // VPNDETAIL_H
