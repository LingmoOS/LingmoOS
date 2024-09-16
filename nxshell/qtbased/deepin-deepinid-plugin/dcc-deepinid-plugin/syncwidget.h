// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "pages/login.h"
#include "pages/logininfo.h"
#include "pages/logininfodetail.h"
#include "operation/syncmodel.h"
#include "operation/syncworker.h"

QT_BEGIN_NAMESPACE
class QWidget;
class QStackedLayout;
class QLabel;
QT_END_NAMESPACE

class SyncWidget : public QWidget
{
    Q_OBJECT
public:
    enum SyncType : int;
    enum SyncState : int;

public:
    explicit SyncWidget(QWidget *parent = nullptr);
    ~SyncWidget();
    void setModel(SyncModel *model);
    void setWorker(SyncWorker *worker);
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void initConnect();

Q_SIGNALS:
    void clearCloudData();
    void requestLoginUser() const;

    void requestAsyncLogoutUser() const;
    void requestSetFullname(const QString &fullname);
    void requestPullMessage() const;

    void requestBindAccount(const QString &uuid, const QString &hostName);
    void requestUnBindAccount(const QString &ubid);
    void requestSetAutoSync(bool enable) const;
    void requestSetModuleState(const QString &syncType, bool state);
    void requestLocalBindCheck(const QString &uuid);

    void requestUOSID(QString &uosid);
    void requestUUID(QString &uuid);
    void requestHostName(QString &hostName);

    void requestSetUtcloudModuleState(const QString &syncType, bool state);
    void refreshDeviceList();
    void refreshSyncList();

    void removeDevice(const QString &devid);
private:
    void onUserInfoChanged(const QVariantMap &userInfo);

private:
    QStackedLayout *m_mainLayout;
    SyncModel *m_model;

    LoginPage *m_loginPage;
    LoginInfoPage *m_loginInfoPage;
    LoginInfoDetailPage *m_loginInfoDetailPage;
    DBackgroundGroup *m_groundGroup;

    bool m_isLogind;
};
