// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCKCONTENT_H
#define LOCKCONTENT_H

#include <QWidget>
#include <QLocalServer>

#include <memory>

#include "mediawidget.h"
#include "sessionbasemodel.h"
#include "sessionbasewindow.h"
#include "systemmonitor.h"

#include "wm_interface.h"

class AuthWidget;
class MFAWidget;
class SFAWidget;
class UserFrameList;
class ControlWidget;
class UserInputWidget;
class User;
class ShutdownWidget;
class LogoWidget;
class TimeWidget;
class QDBusInterface;

class LockContent : public SessionBaseWindow
{
    Q_OBJECT

public:
    explicit LockContent(SessionBaseModel *const model, QWidget *parent = nullptr);

    virtual void onCurrentUserChanged(std::shared_ptr<User> user);
    virtual void onStatusChanged(SessionBaseModel::ModeStatus status);
    virtual void restoreMode();
    void updateGreeterBackgroundPath(const QString &path);
    void updateDesktopBackgroundPath(const QString &path);

signals:
    void requestBackground(const QString &path);
    void requestSwitchToUser(std::shared_ptr<User> user);
    void requestSetKeyboardLayout(std::shared_ptr<User> user, const QString &value);

    void requestStartAuthentication(const QString &account, const int authType);
    void sendTokenToAuth(const QString &account, const int authType, const QString &token);
    void requestEndAuthentication(const QString &account, const int authType);
    void authFinished();

    void requestCheckAccount(const QString &account);
    void requestLockFrameHide();

public slots:
    void pushPasswordFrame();
    void pushUserFrame();
    void pushConfirmFrame();
    void pushShutdownFrame();
    void setMPRISEnable(const bool state);
    void onNewConnection();
    void onDisConnect();

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

protected:
    void updateTimeFormat(bool use24);
    void toggleModule(const QString &name);
    void onUserListChanged(QList<std::shared_ptr<User>> list);
    void tryGrabKeyboard();
    void hideToplevelWindow();
    void currentWorkspaceChanged();
    void updateWallpaper(const QString &path);
    void refreshBackground(SessionBaseModel::ModeStatus status);
    void refreshLayout(SessionBaseModel::ModeStatus status);

    void initUI();
    void initConnections();
    void initMFAWidget();
    void initSFAWidget();
    void initUserListWidget();

private slots:
    void onValueChanged(const QDBusMessage &dbusMessage);

private:
    QString configPath(std::shared_ptr<User>) const;
    QString regionValue(const QString& key) const;
    void buildConnect();
    void disconnect(std::shared_ptr<User> user);

protected:
    SessionBaseModel *m_model;
    ControlWidget *m_controlWidget;                 // 右下角图标
    QSharedPointer<ShutdownWidget> m_shutdownFrame; // 关机界面
    LogoWidget *m_logoWidget;                       // logo显示
    TimeWidget *m_timeWidget;                       // 时间日期显示
    QSharedPointer<MediaWidget> m_mediaWidget;      // 多媒体信息显示
    UserFrameList *m_userListWidget;                // 账户展示区域

    std::shared_ptr<User> m_user;
    QList<QMetaObject::Connection> m_currentUserConnects;
    com::deepin::wm *m_wmInter;                     // 用户更新工作区壁纸

    SFAWidget *m_sfaWidget;
    MFAWidget *m_mfaWidget;
    AuthWidget *m_authWidget;

    int m_lockFailTimes;                             // 尝试锁屏时失败的次数
    QLocalServer *m_localServer;

    QString m_localeName;
    QString m_shortTimeFormat;
    QString m_longDateFormat;
};

#endif // LOCKCONTENT_H
