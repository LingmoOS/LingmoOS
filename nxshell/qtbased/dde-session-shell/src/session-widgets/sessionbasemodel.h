// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONBASEMODEL_H
#define SESSIONBASEMODEL_H

#include "authcommon.h"
#include "userinfo.h"
#include "mfainfolist.h"

#include <DSysInfo>

#include <QObject>

#include <memory>

using namespace AuthCommon;

class SessionBaseModel : public QObject
{
    Q_OBJECT
public:
    enum AuthFailedType {
        Fprint,
        KEYBOARD
    };
    Q_ENUM(AuthFailedType)

    enum PowerAction {
        None,
        RequireNormal,
        RequireShutdown,
        RequireRestart,
        RequireSuspend,
        RequireHibernate,
        RequireLock,
        RequireLogout,
        RequireSwitchUser,
        RequireSwitchSystem
    };
    Q_ENUM(PowerAction)

    enum ModeStatus {
        NoStatus,
        PasswordMode,           // 输入密码验证页面
        ConfirmPasswordMode,    // 确认密码页面(一般是关机或者重启等操作需要验证用户密码)
        UserMode,               // 显示用户列表
        PowerMode,
        ShutDownMode,           // 关机界面
        ResetPasswdMode         // 重设密码界面
    };
    Q_ENUM(ModeStatus)

    /* org.deepin.dde.Authenticate1 */
    struct AuthProperty {
        bool FuzzyMFA;          // Reserved
        bool MFAFlag;           // 多因子标志位
        int FrameworkState;     // 认证框架是否可用标志位
        int AuthType;           // 账户开启的认证类型
        int MixAuthFlags;       // 受支持的认证类型
        int PINLen;             // PIN 码的最大长度
        QString EncryptionType; // 加密类型
        QString Prompt;         // 提示语
        QString UserName;       // 账户名
    };

    explicit SessionBaseModel(QObject *parent = nullptr);
    ~SessionBaseModel() override;

    inline std::shared_ptr<User> currentUser() const { return m_currentUser; }
    inline std::shared_ptr<User> lastLogoutUser() const { return m_lastLogoutUser; }

    inline QList<std::shared_ptr<User>> loginedUserList() const { return m_loginedUsers->values(); }
    inline QList<std::shared_ptr<User>> userList() const { return m_users->values(); }

    std::shared_ptr<User> findUserByUid(const uint uid) const;
    std::shared_ptr<User> findUserByName(const QString &name) const;

    inline AppType appType() const { return m_appType; }
    void setAppType(const AppType type);

    void setSEType(bool state) { m_SEOpen = state; }
    bool getSEType() const { return m_SEOpen; }

    inline QString sessionKey() const { return m_sessionKey; }
    void setSessionKey(const QString &sessionKey);

    inline PowerAction powerAction() const { return m_powerAction; }
    void setPowerAction(const PowerAction &powerAction);

    ModeStatus currentModeState() const { return m_currentModeState; }
    void setCurrentModeState(const ModeStatus &currentModeState);

    inline int userListSize() { return m_userListSize; }
    void setUserListSize(int users_size);

    void setHasVirtualKB(bool hasVirtualKB);

    void setHasSwap(bool hasSwap);
    inline bool hasSwap() { return m_hasSwap; }

    inline bool isUseWayland() const { return m_isUseWayland; }

    inline bool visible() const { return m_visible; }
    void setVisible(const bool visible);

    inline bool canSleep() const { return m_canSleep; }
    void setCanSleep(bool canSleep);

    inline bool allowShowUserSwitchButton() const { return m_allowShowUserSwitchButton; }
    void setAllowShowUserSwitchButton(bool allowShowUserSwitchButton);

    inline bool alwaysShowUserSwitchButton() const { return m_alwaysShowUserSwitchButton; }
    void setAlwaysShowUserSwitchButton(bool alwaysShowUserSwitchButton);

    inline bool isServerModel() const { return m_isServerModel; }
    void setIsServerModel(bool server_model);

    inline bool abortConfirm() const { return m_abortConfirm; }
    void setAbortConfirm(bool abortConfirm);

    inline bool isBlackMode() const { return m_isBlackMode; }
    void setIsBlackMode(bool is_black);

    inline bool isHibernateMode() const { return m_isHibernateMode; }
    void setIsHibernateModel(bool is_Hibernate);

    inline bool isCheckedInhibit() const { return m_isCheckedInhibit; }
    void setIsCheckedInhibit(bool checked);

    inline bool allowShowCustomUser() const { return m_allowShowCustomUser; }
    void setAllowShowCustomUser(const bool allowShowCustomUser);

    inline const QList<std::shared_ptr<User>> getUserList() const { return m_users->values(); }

    inline const AuthProperty &getAuthProperty() const { return m_authProperty; }
    void setAuthType(const int type);

    std::shared_ptr<User> json2User(const QString &userJson);

signals:
    /* org.deepin.dde.Accounts1 */
    void currentUserChanged(const std::shared_ptr<User>);
    void userAdded(const std::shared_ptr<User>);
    void userRemoved(const std::shared_ptr<User>);
    void userListChanged(const QList<std::shared_ptr<User>>);
    void loginedUserListChanged(const QList<std::shared_ptr<User>>);
    /* org.deepin.dde.Authenticate1 */
    void MFAFlagChanged(const bool);
    /* others */
    void visibleChanged(const bool);

public slots:
    /* org.deepin.dde.Accounts1 */
    void addUser(const QString &path);
    void addUser(const std::shared_ptr<User> user);
    void removeUser(const QString &path);
    void removeUser(const std::shared_ptr<User> user);
    bool updateCurrentUser(const QString &userJson);
    bool updateCurrentUser(const std::shared_ptr<User> user);
    void updateUserList(const QStringList &list);
    void updateLastLogoutUser(const uid_t uid);
    void updateLastLogoutUser(const std::shared_ptr<User> lastLogoutUser);
    void updateLoginedUserList(const QString &list);
    /* org.deepin.dde.Authenticate1 */
    void updateLimitedInfo(const QString &info);
    void updateFrameworkState(const int state);
    void updateSupportedMixAuthFlags(const int flags);
    void updateSupportedEncryptionType(const QString &type);
    /* org.deepin.dde.Authenticate1.Session */
    void updateAuthState(const int type, const int state, const QString &message);
    void updateFactorsInfo(const MFAInfoList &infoList);
    void updateFuzzyMFA(const bool fuzzMFA);
    void updateMFAFlag(const bool MFAFlag);
    void updatePINLen(const int PINLen);
    void updatePrompt(const QString &prompt);

signals:
    void authTipsMessage(const QString &message, AuthFailedType type = KEYBOARD);
    void authFailedMessage(const QString &message, AuthFailedType type = KEYBOARD);
    void authFailedTipsMessage(const QString &message, AuthFailedType type = KEYBOARD);
    void authFinished(bool success);
    void onPowerActionChanged(PowerAction poweraction);
    void onRequirePowerAction(PowerAction poweraction, bool needConfirm);
    void onSessionKeyChanged(const QString &sessionKey);
    void showUserList();
    void showLockScreen();
    void showShutdown();
    void onStatusChanged(ModeStatus status);
    void onUserListChanged(QList<std::shared_ptr<User>> list);
    void hasVirtualKBChanged(bool hasVirtualKB);
    void onUserListSizeChanged(int users_size);
    void onHasSwapChanged(bool hasSwap);
    void canSleepChanged(bool canSleep);
    void allowShowUserSwitchButtonChanged(bool allowShowUserSwitchButton);
    void abortConfirmChanged(bool abortConfirm);
    void userListLoginedChanged(QList<std::shared_ptr<User>> list);
    void activeAuthChanged(bool active);
    void blackModeChanged(bool is_black);
    void HibernateModeChanged(bool is_hibernate); //休眠信号改变
    void prepareForSleep(bool is_Sleep);          //待机信号改变
    void shutdownInhibit(const SessionBaseModel::PowerAction action, bool needConfirm);
    void cancelShutdownInhibit(bool hideFrame);
    void requestLoginFrame();
    void clearServerLoginWidgetContent();

    void authStateChanged(const int, const int, const QString &);
    void authTypeChanged(const int type);

private:
    bool m_hasSwap;
    bool m_visible;
    bool m_isServerModel;
    bool m_canSleep;
    bool m_allowShowUserSwitchButton;
    bool m_alwaysShowUserSwitchButton;
    bool m_abortConfirm;
    bool m_isLockNoPassword;
    bool m_isBlackMode;
    bool m_isHibernateMode;
    bool m_isLock;
    bool m_allowShowCustomUser;
    bool m_SEOpen;                                  // 保存等保开启、关闭的状态
    bool m_isUseWayland;
    int m_userListSize;
    AppType m_appType;
    QList<std::shared_ptr<User>> m_userList;
    std::shared_ptr<User> m_currentUser;
    std::shared_ptr<User> m_lastLogoutUser;
    QString m_sessionKey;
    PowerAction m_powerAction;
    ModeStatus m_currentModeState;
    bool m_isCheckedInhibit;
    AuthProperty m_authProperty;                    // 认证相关属性的值，初始时通过dbus获取，暂存在model中，供widget初始化界面使用
    QMap<QString, std::shared_ptr<User>> *m_users;
    QMap<QString, std::shared_ptr<User>> *m_loginedUsers;
};

#endif // SESSIONBASEMODEL_H
