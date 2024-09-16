// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockframe.h"

#include "lockcontent.h"
#include "sessionbasemodel.h"
#include "userinfo.h"
#include "warningcontent.h"
#include "public_func.h"

#include <DDBusSender>

#include <QApplication>
#include <QDBusInterface>
#include <QGSettings>
#include <QScreen>
#include <QWindow>
#include <QX11Info>

xcb_atom_t internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, static_cast<uint16_t>(strlen(name)), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

LockFrame::LockFrame(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(model, parent)
    , m_model(model)
    , m_lockContent(new LockContent(model))
    , m_warningContent(nullptr)
    , m_enablePowerOffKey(false)
    , m_autoExitTimer(nullptr)
{
    xcb_connection_t *connection = QX11Info::connection();
    if (connection) {
        xcb_atom_t cook = internAtom(connection, "_DEEPIN_LOCK_SCREEN", false);
        xcb_change_property(connection, XCB_PROP_MODE_REPLACE, static_cast<xcb_window_t>(winId()),
                            cook, XCB_ATOM_ATOM, 32, 1, &cook);
    }

    updateBackground(m_model->currentUser()->greeterBackground());

    setAccessibleName("LockFrame");
    m_lockContent->setAccessibleName("LockContent");
    m_lockContent->hide();
    setContent(m_lockContent);

    connect(m_lockContent, &LockContent::requestSwitchToUser, this, &LockFrame::requestSwitchToUser);
    connect(m_lockContent, &LockContent::requestSetKeyboardLayout, this, &LockFrame::requestSetKeyboardLayout);
    connect(m_lockContent, &LockContent::requestBackground, this, static_cast<void (LockFrame::*)(const QString &)>(&LockFrame::updateBackground));
    connect(m_lockContent, &LockContent::requestStartAuthentication, this, &LockFrame::requestStartAuthentication);
    connect(m_lockContent, &LockContent::sendTokenToAuth, this, &LockFrame::sendTokenToAuth);
    connect(m_lockContent, &LockContent::requestEndAuthentication, this, &LockFrame::requestEndAuthentication);
    connect(m_lockContent, &LockContent::requestLockFrameHide, this, &LockFrame::hide);
    connect(m_lockContent, &LockContent::authFinished, this, [this] {
        hide();
        emit requestEnableHotzone(true);
        emit authFinished();
    });
    connect(m_lockContent, &LockContent::requestCheckAccount, this, &LockFrame::requestCheckAccount);
    connect(model, &SessionBaseModel::showUserList, this, &LockFrame::showUserList);
    connect(model, &SessionBaseModel::showLockScreen, this, &LockFrame::showLockScreen);
    connect(model, &SessionBaseModel::showShutdown, this, &LockFrame::showShutdown);
    connect(model, &SessionBaseModel::shutdownInhibit, this, &LockFrame::shutdownInhibit);
    connect(model, &SessionBaseModel::cancelShutdownInhibit, this, &LockFrame::cancelShutdownInhibit);
    connect(model, &SessionBaseModel::prepareForSleep, this, [ = ](bool isSleep) {
        //不管是待机还是唤醒均不响应电源按键信号
        m_enablePowerOffKey = false;

        //唤醒1秒后才响应电源按键信号，避免待机唤醒时响应信号，将界面切换到关机选项
        if (!isSleep) {
            QTimer::singleShot(1000, this, [ = ] {
                m_enablePowerOffKey = true;
            });
        }

        //待机时由锁屏提供假黑屏，唤醒时显示正常界面
        model->setIsBlackMode(isSleep);
        model->setVisible(true);

        if (!isSleep) {
            //待机唤醒后检查是否需要密码，若不需要密码直接隐藏锁定界面
            if (QGSettings::isSchemaInstalled("com.deepin.dde.power")) {
                QGSettings powerSettings("com.deepin.dde.power", QByteArray(), this);
                if (!powerSettings.get("sleep-lock").toBool()) {
                    hide();
                }
            }
        }
    } );

    connect(model, &SessionBaseModel::authFinished, this, [this](bool success) {
        if (success) {
            Q_EMIT requestEnableHotzone(true);
            hide();
        }
    });

    //程序启动1秒后才响应电源按键信号，避免第一次待机唤醒启动锁屏程序后响应信号，将界面切换到关机选项
    QTimer::singleShot(1000, this, [ = ] {
        m_enablePowerOffKey = true;
    });

    if (getDConfigValue(getDefaultConfigFileName(), "autoExit", false).toBool()) {
        m_autoExitTimer = new QTimer(this);
        m_autoExitTimer->setInterval(1000*60); //1分钟
        m_autoExitTimer->setSingleShot(true);
        connect(m_autoExitTimer, &QTimer::timeout, qApp, &QApplication::quit);
    }
}

bool LockFrame::event(QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QString  keyValue = "";
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_PowerOff: {
            if (!handlePoweroffKey()) {
                keyValue = "power-off";
            }
            break;
        }
        case Qt::Key_NumLock: {
            keyValue = "numlock";
            break;
        }
        case Qt::Key_TouchpadOn: {
            keyValue = "touchpad-on";
            break;
        }
        case Qt::Key_TouchpadOff: {
            keyValue = "touchpad-off";
            break;
        }
        case Qt::Key_TouchpadToggle: {
            keyValue = "touchpad-toggle";
            break;
        }
        case Qt::Key_CapsLock: {
            keyValue = "capslock";
            break;
        }
        case Qt::Key_VolumeDown: {
            keyValue = "audio-lower-volume";
            break;
        }
        case Qt::Key_VolumeUp: {
            keyValue = "audio-raise-volume";
            break;
        }
        case Qt::Key_VolumeMute: {
            keyValue = "audio-mute";
            break;
        }
        case Qt::Key_MonBrightnessUp: {
            keyValue = "mon-brightness-up";
            break;
        }
        case Qt::Key_MonBrightnessDown: {
            keyValue = "mon-brightness-down";
            break;
        }
        }

        if (keyValue != "") {
            emit sendKeyValue(keyValue);
        }
    }
    return FullscreenBackground::event(event);
}

void LockFrame::resizeEvent(QResizeEvent *event)
{
    m_lockContent->resize(size());
    FullscreenBackground::resizeEvent(event);
}

bool LockFrame::handlePoweroffKey()
{
    QDBusInterface powerInter("org.deepin.dde.Power1","/org/deepin/dde/Power1","org.deepin.dde.Power1");
    if (!powerInter.isValid()) {
        qDebug() << "powerInter is not valid";
        return false;
    }
    bool isBattery = powerInter.property("OnBattery").toBool();
    int action = 0;
    if (isBattery) {
        action = powerInter.property("BatteryPressPowerBtnAction").toInt();
    } else {
        action = powerInter.property("LinePowerPressPowerBtnAction").toInt();
    }
    qDebug() << "battery is: " << isBattery << "," << action;
    // 需要特殊处理：关机(0)和无任何操作(4)
    if (action == 0) {
        //锁屏时或显示关机界面时，需要确认是否关机
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
        return true;
    } else if (action == 4) {
        // 先检查当前是否允许响应电源按键
        if (m_enablePowerOffKey && m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode) {
            //无任何操作时，如果是锁定时显示小关机界面
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
        }
        return true;
    }
    return false;
}

void LockFrame::showUserList()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    QTimer::singleShot(10, this, [ = ] {
        this->show();
    });
}

void LockFrame::showLockScreen()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    show();
}

void LockFrame::showShutdown()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    show();
}

void LockFrame::shutdownInhibit(const SessionBaseModel::PowerAction action, bool needConfirm)
{
    //如果其他显示屏界面已经检查过是否允许关机，此显示屏上的界面不再显示，避免重复检查并触发信号
    if (m_model->isCheckedInhibit()) return;
    //记录多屏状态下当前显示屏是否显示内容
    bool old_visible = contentVisible();

    if (!m_warningContent) {
        m_warningContent = new WarningContent(m_model, action, this);
        m_warningContent->setAccessibleName("WarningContent");
    } else {
        m_warningContent->setPowerAction(action);
    }
    m_warningContent->resize(size());
    setContent(m_warningContent);

    //多屏状态下，当前界面显示内容时才显示提示界面
    if (old_visible) {
        setContentVisible(true);
        m_lockContent->hide();
    }

    //检查是否允许关机
    m_warningContent->beforeInvokeAction(needConfirm);
}

void LockFrame::cancelShutdownInhibit(bool hideFrame)
{
    //允许关机检查结束后切换界面
    //记录多屏状态下当前显示屏是否显示内容
    bool old_visible = contentVisible();

    setContent(m_lockContent);

    //隐藏提示界面
    if (m_warningContent) {
        m_warningContent->hide();
    }

    //多屏状态下，当前界面显示内容时才显示
    if (old_visible) {
        setContentVisible(true);
    }

    if (hideFrame) {
        m_model->setVisible(false);
        m_model->setCurrentModeState(SessionBaseModel::PasswordMode);
    }
}

void LockFrame::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();       break;
#endif
    }
}

void LockFrame::showEvent(QShowEvent *event)
{
    emit requestEnableHotzone(false);

    m_model->setVisible(true);
    if (m_autoExitTimer)
        m_autoExitTimer->stop();

    return FullscreenBackground::showEvent(event);
}

void LockFrame::hideEvent(QHideEvent *event)
{
    emit requestEnableHotzone(true);

    m_model->setVisible(false);
    if (m_autoExitTimer)
        m_autoExitTimer->start();

    return FullscreenBackground::hideEvent(event);
}
