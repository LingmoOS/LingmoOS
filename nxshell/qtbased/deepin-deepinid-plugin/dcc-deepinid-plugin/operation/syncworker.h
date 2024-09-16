// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCWORKER_H
#define SYNCWORKER_H

#include "syncmodel.h"

#include "operation/deepiniddbusproxy.h"
#include "operation/syncdbusproxy.h"

#include <QObject>

struct BindCheckResult {
    QString ubid = "";
    QString error = "";
    bool ret = false;
};

class SyncWorker : public QObject
{
    Q_OBJECT
public:
    explicit SyncWorker(SyncModel * model, QObject *parent = nullptr);

    void initData();
    void activate();
    void deactivate();
    void refreshSwitcherDump(); // 拉取utcloud.Daemon 解析json数据
    bool checkPasswdEmpty(bool &isEmpty);
    QString getSessionID();

public Q_SLOTS:
    void setSync(std::pair<SyncType, bool> state);
    void setSyncState(const QString &syncType, bool state);
    void setUtcloudState(const QString &utcloudType, bool state);
    void loginUser();
    void logoutUser();
    void asyncLogoutUser(const QString &ubid);
    void setAutoSync(bool autoSync);
    void licenseStateChangeSlot();

    void getUOSID();
    void getUUID();
    void getHostName();

    void getRSAPubKey();
    void getDeviceList();
    void clearData();
    void unBindPlatform();
    void refreshUserInfo();
    void refreshSyncInfo();

    void openForgetPasswd(const QString &url);
    void registerPasswd(const QString &strpwd);
    void removeDevice(const QString &devid);
    bool checkPassword(const QString &passwd, QString &encryptPwd, int &count);
    //oldpwd is encrypted already, newpwd is not encrypted
    bool resetPassword(const QString &oldpwd, const QString &newpwd);
    int sendVerifyCode(const QString &straddr);
    bool updatePhoneEmail(const QString &phonemail, const QString &verifyCode,
                          QString &rebindAccount, QString &rebindKey);

    void asyncLocalBindCheck(const QString &uuid);
    void asyncBindAccount(const QString &uuid, const QString &hostName);
    void asyncUnbindAccount(const QString &ubid);

    void onSetFullname(const QString &fullname);
    void asyncSetFullname(const QString &fullname);
    /**
     * @brief onPullMessage  插件打开 若是登录状态 用户停留5秒以上 进行同步操作
     */
    void onPullMessage();
    void futurePullMessage();

    // 相应回调后接口函数
    void callWatcherResult(const QString &result);
    void refreshSyncState();

    void utcloudSwitcherChange(QVariantList data);
    void utcloudLoginStatus(QVariantList data);

signals:
    void userDataChanged(const IntString& state);
    void pendingCallWatcherFinished(const QString& pendingResult);
    void SwitcherChange(const QString&, bool);

private:
    void getUserDeepinidInfo();

    void getTrustDeviceList(int pageIndex = 1, int pageSize = 100);
private:
    void onSyncModuleStateChanged(const QString& module, bool enable);
    void onStateChanged(const IntString& state);
    void onLastSyncTimeChanged(qlonglong lastSyncTime);
    void getLicenseState();
    BindCheckResult checkLocalBind(const QString &uuid);
    BindCheckResult logout(const QString &uuid);
    BindCheckResult bindAccount(const QString &uuid, const QString &hostName);
    BindCheckResult unBindAccount(const QString &ubid);

private:
    SyncModel *m_model;
    SyncDaemon *m_syncInter;
    DeepinIdProxy *m_deepinId_inter;
    QDBusInterface *m_syncHelperInter;
    QDBusInterface *m_utcloudInter;
    QFileSystemWatcher *m_watcher;
    std::string m_RSApubkey;
    QString m_pwdToken;
};

#endif // SYNCWORKER_H
