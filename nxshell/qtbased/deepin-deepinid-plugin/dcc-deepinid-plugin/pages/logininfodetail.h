// Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "interface/namespace.h"

#include "operation/syncmodel.h"
#include "operation/syncworker.h"
#include "utils.h"
#include "syncstateicon.h"

#include <dtkwidget_global.h>
#include <widgets/settingsgroup.h>
#include <widgets/settingsitem.h>
#include <polkit-qt5-1/PolkitQt1/Authority>
#include "cloudsync.h"
#include "devicesync.h"
#include "securitymanage.h"
#include <QStackedLayout>

using namespace PolkitQt1;

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DTipLabel;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QObject;
class QWidget;
class QVBoxLayout;
class QStandardItemModel;
class QStandardItem;
class QLabel;
QT_END_NAMESPACE

namespace dcc {
namespace widgets {
class SwitchWidget;
}
}


class SettingsItemEx : public DCC_NAMESPACE::SettingsItem
{
    Q_OBJECT
public:
    explicit SettingsItemEx(QWidget *parent = nullptr)
        : SettingsItem(parent)
        , m_listView(nullptr) {

    }

    inline void setListView(DListView * listView) {
        m_listView = listView;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    DListView * m_listView;
};

/**
 * @brief The LoginInfoDetailPage class 展示登录信息三级页面
 */
class LoginInfoDetailPage : public QWidget
{
    Q_OBJECT
public:
    enum InfoType {
        Available,
        NoService,  // 无服务
        NoNetwork,
        NoActive // 未激活
    };

public:
    explicit LoginInfoDetailPage(QWidget *parent = nullptr);
    ~LoginInfoDetailPage();

    void setModel(SyncModel *model);

    void setWorker(SyncWorker *worker);

    void initUI();
    void initConnection();

    void onLogin();
Q_SIGNALS:
    void onUserLogout();
    void clearCloudData();
    void requestBindAccount(const QString &uuid, const QString &hostName);
    void requestUnBindAccount(const QString &ubid);

    void requestSetAutoSync(bool enable) const;
    void requestSetModuleState(const QString &syncType, bool state);
    void requestLocalBindCheck(const QString &uuid);

    void requestSetUtcloudModuleState(const QString &utcloudType, bool state);

    // 请求在浏览器打开账户中心
    void requestOpenWeb();
    // DELETE
    void requestUOSID(QString &uosid);
    void requestUUID(QString &uuid);
    void selectIndex(int index);
    void removeDevice(const QString &devid);

public Q_SLOTS:
    void updateItemCheckStatus(const QString &name, bool visible);
    void onModuleStateChanged(std::pair<SyncType, bool> state);

    void onStateChanged(const std::pair<qint32, QString> &state);

    void onCheckAuthorizationFinished(Authority::Result authResult);
    void onListViewClicked(const QModelIndex &index);

private:
    void onBindUserAccountChanged(bool checked);
    void showItemDisabledStatus(InfoType status);
    void SyncTimeLblVisible(bool isVisible);

private:
    SyncModel *m_model;
    QStackedLayout *m_mainLayout;
    // 同步配置项
    DTK_WIDGET_NAMESPACE::DListView *m_listView;
    QStandardItemModel *m_listModel;
    QMap<SyncType, QStandardItem *> m_itemMap;
    SyncStateIcon *m_stateIcon;

    // 最后同步时间
    QLabel *m_lastSyncTimeLbl;
    DTipLabel *m_disabledTips;
    SettingsItemEx *m_syncHead;


    Authority * m_authority;
    CloudSyncPage *m_cloudPage;
    DeviceSyncPage *m_devPage;
    SecurityPage *m_securityPage;
};
