/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef VPN_H
#define VPN_H

#include <QObject>
#include <QtPlugin>
#include <QtDBus/QDBusInterface>
#include <QFrame>
#include <QLabel>

#include "interface.h"
#include "addbtn.h"
#include "imageutil.h"
#include "kwidget.h"
#include "kswitchbutton.h"
#include "itemframe.h"
#include "ukcccommon.h"

using namespace kdk;
using namespace ukcc;

namespace Ui {
class Vpn;
}

class Vpn : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Vpn();
    ~Vpn();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    QString translationPath() const  Q_DECL_OVERRIDE;

public:
    void initComponent();
    void initConnect();

    void runExternalApp();

protected:
//    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::Vpn *ui;

    QString m_pluginName;
    int m_pluginType;
    QWidget * m_pluginWidget;

    QDBusInterface  *m_interface = nullptr;

    QFrame *m_topFrame;
    QFrame *m_showFrame;
    QLabel *m_showLabel;
    KSwitchButton *m_showBtn;
    QFrame *m_Line;
    QFrame *m_timeFrame;
    QLabel *m_timeLabel;
    KSwitchButton *m_timeBtn;
    ItemFrame *m_listFrame;

    bool m_firstLoad;
    QGSettings  *m_switchGsettings;

    QFrame* myLine();

    int getInsertPos(QString connName);

    void deleteVpn(QString uuid);
    void activeConnect(QString uuid);
    void deActiveConnect(QString uuid);
    void showDetailPage(QString uuid);

    //获取设备列表
    void initNet();
    //增加一项
    void addOneVirtualItem(QStringList infoList);
    //减少一项
    void removeOneVirtualItem(QString uuid);
    //单个lan连接状态变化
    void itemActiveConnectionStatusChanged(VpnItem *item, int status);

    void setShowSwitchStatus();
    // 搜索词条
    void initSearchText();

private slots:
    void onVpnAdd(QStringList);
    void onVpnRemove(QString);
    void onVpnUpdate(QStringList);
    void onVpnActiveConnectionStateChanged(QString, int);

};

#endif // VPN_H
