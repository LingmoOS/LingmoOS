// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICESYNC_H
#define DEVICESYNC_H

#include "interface/namespace.h"
#include <QWidget>
#include <dtkwidget_global.h>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QListView>
#include <DListView>
#include <polkit-qt5-1/PolkitQt1/Authority>
#include "operation/syncmodel.h"
#include "cloudsync.h"
#include "widgets/switchwidget.h"

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DLabel;
class DLineEdit;
class DToolButton;
class DDialog;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

using namespace PolkitQt1;

class DevListView:public DListView
{
public:
    explicit DevListView(QWidget *parent = nullptr);
protected:
    bool event(QEvent *e) override;
private:
    void hoverEnterEvent(const QPoint &p);

    void hoverLeaveEvent();
};


class DeviceSyncPage : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceSyncPage(QWidget *parent = nullptr);

    void setSyncModel(SyncModel *syncModel);

    void onLogin();
Q_SIGNALS:
    void onUserLogout();

    void bindChanged(bool checked);

    void removeDevice(const QString &devid);

    void requestLocalBindCheck(const QString &uuid);

    void requestBindAccount(const QString &uuid, const QString &hostName);

    void requestUnBindAccount(const QString &ubid);
public Q_SLOTS:
    void onUserUnbindInfoChanged(const QString& ubid);

    void onBindStateChanged(bool state);

    void onUserInfoChanged(const QVariantMap &infos);

    void onCheckAuthorizationFinished(Authority::Result authResult);

    void onAddDeviceList(const QList<DeviceInfo> &devlist);
private:
    void initUI();

    void initConnection();

    void updateUserBindStatus();

    void makeContentDisable(bool bEnable = false);

    void initDelDialog();

    QString getDeviceIcon(DeviceType devType);

    void bindUserAccount();

    void unbindUserAccount();
private:
    DCC_NAMESPACE::SwitchWidget *m_bindSwitch;
    DevListView *m_devList;
    QStandardItemModel *m_devModel;
    DDialog *m_delDialog;
    SyncModel *m_syncModel;
    WarnLabel *m_labelWarn;
    // 解绑ID
    QString m_ubID;
    QString m_currDevice;
};

#endif // DEVICESYNC_H
