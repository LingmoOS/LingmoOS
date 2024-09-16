// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCKFRAME
#define LOCKFRAME

#include "fullscreenbackground.h"
#include "sessionbasemodel.h"

#include <QKeyEvent>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QTimer>

#include <memory>

const QString DBUS_LOCK_PATH = "/org/deepin/dde/LockFront1";
const QString DBUS_LOCK_NAME = "org.deepin.dde.LockFront1";

const QString DBUS_SHUTDOWN_PATH = "/org/deepin/dde/ShutdownFront1";
const QString DBUS_SHUTDOWN_NAME = "org.deepin.dde.ShutdownFront1";

class DBusLockService;
class LockContent;
class WarningContent;
class User;
class LockFrame: public FullscreenBackground
{
    Q_OBJECT
public:
    LockFrame(SessionBaseModel *const model, QWidget *parent = nullptr);

signals:
    void requestSwitchToUser(std::shared_ptr<User> user);
    void requestSetKeyboardLayout(std::shared_ptr<User> user, const QString &layout);
    void requestEnableHotzone(bool disable);
    void sendKeyValue(QString keyValue);

    void requestStartAuthentication(const QString &account, const int authType);
    void sendTokenToAuth(const QString &account, const int authType, const QString &token);
    void requestEndAuthentication(const QString &account, const int authType);
    void authFinished();
    void requestCheckAccount(const QString &account);

public slots:
    void showUserList();
    void showLockScreen();
    void showShutdown();
    void shutdownInhibit(const SessionBaseModel::PowerAction action, bool needConfirm);
    void cancelShutdownInhibit(bool hideFrame);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    bool handlePoweroffKey();

private:
    SessionBaseModel *m_model;
    LockContent *m_lockContent;
    WarningContent *m_warningContent;
    bool m_enablePowerOffKey;
    QTimer *m_autoExitTimer;
};

#endif // LOCKFRAME
