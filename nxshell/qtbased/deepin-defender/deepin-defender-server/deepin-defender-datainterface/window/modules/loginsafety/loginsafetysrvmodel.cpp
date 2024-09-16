// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginsafetysrvmodel.h"

#include <QDBusInterface>
#include <QGSettings>
#include <QtConcurrent>

#include <pwd.h>
#include <unistd.h>

LoginSafetySrvModel::LoginSafetySrvModel(QObject *parent)
    : QObject(parent)
    , m_gSettings(new QGSettings("com.deepin.dde.deepin-defender", QByteArray(), this))
    , m_pwdConfInter(new QDBusInterface("com.deepin.daemon.PasswdConf",
                                        "/com/deepin/daemon/PasswdConf",
                                        "com.deepin.daemon.PasswdConf",
                                        QDBusConnection::systemBus(), this))
    , m_accountInter(new AccountInter("com.deepin.daemon.Accounts",
                                      "/com/deepin/daemon/Accounts",
                                      QDBusConnection::systemBus(), this))
{
    checkPwdLastChangeDays();
}

LoginSafetySrvModel::~LoginSafetySrvModel()
{
}

// 获取密码校验策略开启关闭状态
bool LoginSafetySrvModel::getPwdLimitPolicyEnable()
{
    // 获取限制策略
    QDBusReply<bool> reply = m_pwdConfInter->call("GetEnabled");
    if (!reply.isValid()) {
        qDebug() << reply.error();
        return false;
    }

    return reply.value();
}

// 设置密码校验策略开启关闭状态
void LoginSafetySrvModel::setPwdLimitPolicyEnable(const bool &enable)
{
    QtConcurrent::run([this, enable] {
        QDBusMessage msg = m_pwdConfInter->call("SetEnabled", enable);
        // 如果调用没有出错，则发送开启/关闭密码限制策略完成信号
        QDBusMessage::MessageType msgType = msg.type();
        if (QDBusMessage::MessageType::ErrorMessage != msgType
            && QDBusMessage::MessageType::InvalidMessage != msgType) {
            // 发送开启/关闭密码限制策略完成信号
            Q_EMIT this->setPwdLimitPolicyEnableFinished(enable);

            // 如果开启限制策略，则根据安全中心密码等级，重新设置限制策略
            if (enable) {
                int level = m_gSettings->get(PWD_LIMIT_LEVEL).toInt();
                this->setPwdLimitedLevel(level);
            }
        }
    });
}

// 设置密码安全等级
void LoginSafetySrvModel::setPwdLimitedLevel(const int &level)
{
    int minLen = 1;
    int requiredPolicyCount = 1;
    switch (static_cast<PwdLimitLevel>(level)) {
    case Medium:
        minLen = 6;
        requiredPolicyCount = 2;
        break;
    case High:
        minLen = 8;
        requiredPolicyCount = 3;
        break;
    default:
        minLen = 1;
        requiredPolicyCount = 1;
        break;
    }

    QString PwdConfSettingStr = PwdConfStr.arg(minLen).arg(requiredPolicyCount);

    QtConcurrent::run([this, PwdConfSettingStr, level] {
        QDBusMessage msg = m_pwdConfInter->call("WriteConfig", PwdConfSettingStr);
        // 如果调用没有出错，则发送设置限制等级完成信号
        QDBusMessage::MessageType msgType = msg.type();
        if (QDBusMessage::MessageType::ErrorMessage != msgType
            && QDBusMessage::MessageType::InvalidMessage != msgType) {
            m_gSettings->set(PWD_LIMIT_LEVEL, level);
            Q_EMIT this->setPwdLimitedLevelFinished(level);
        }
    });
}

// 获取密码修改错误提示文字
QString LoginSafetySrvModel::getPwdChangeError() const
{
    QString result;
    int pwdLimitLevel = m_gSettings->get(PWD_LIMIT_LEVEL).toInt();
    switch (static_cast<PwdLimitLevel>(pwdLimitLevel)) {
    case High:
        result = tr("The password must have at least 8 characters, and contain at least 3 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols");
        break;
    case Medium:
        result = tr("The password must have at least 6 characters, and contain at least 2 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols");
        break;
    default:
        break;
    }
    return result;
}

// 距上次密码修改的天数
qint64 LoginSafetySrvModel::daysToPasswdLastChanged()
{
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    QString m_currentUserName = QString(pwd->pw_name);
    // 获取当前用户dbus路径
    QString userDbusPath = m_accountInter->FindUserByName(m_currentUserName);
    // 创建接口
    QDBusInterface *accountUserInter = new QDBusInterface("com.deepin.daemon.Accounts",
                                                          userDbusPath, "com.deepin.daemon.Accounts.User",
                                                          QDBusConnection::systemBus(), this);
    // 获取上次密码修改日期距原始日期的天数
    qint64 pwdChangedDaysToOrigin = accountUserInter->property("PasswordLastChange").toLongLong();
    // 1970年1月1日的时间数据
    QDateTime originDateTime = QDateTime(QDate(1970, 1, 1));
    qint64 currentDaysToOrigin = originDateTime.daysTo(QDateTime::currentDateTime());

    // 完成数据获取后，释放指针
    accountUserInter->deleteLater();

    // 返回当前距上次修改密码日期天数
    return currentDaysToOrigin - pwdChangedDaysToOrigin;
}

// 获取密码修改提醒截止天数
qint64 LoginSafetySrvModel::getPwdChangeDeadline()
{
    qint64 deadline = 0; // 修改截止天数为0时，表示永远不会提示密码修改提醒
    switch (m_gSettings->get(PWD_CHANGE_DEADLINE_TYPE).toInt()) {
    case 0:
        deadline = 30;
        break;
    case 1:
        deadline = 60;
        break;
    case 2:
        deadline = 90;
        break;
    case 3:
        deadline = 120;
        break;
    case 4:
        deadline = 150;
        break;
    case 5:
        deadline = 180;
        break;
    case 6:
        deadline = 0;
        break;
    default:
        deadline = 0;
        break;
    }
    return deadline;
}

// 显示密码修改提醒
void LoginSafetySrvModel::showNeedChangePwdSysNotify(qint64 days)
{
    QDBusMessage m = QDBusMessage::createMethodCall("com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification", "Notify");
    QStringList action;
    action << "_open1" << tr("Change password"); //添加按钮
    QVariantMap inform; //按钮的点击操作
    QString openSysSafetyPageCmd = QString("qdbus,com.deepin.dde.ControlCenter,"
                                           "/com/deepin/dde/ControlCenter,"
                                           "com.deepin.dde.ControlCenter.ShowPage,"
                                           "%1,")
                                       .arg("accounts");

    inform.insert(QString("x-deepin-action-_open1"), openSysSafetyPageCmd);
    m << QString("deepin-defender")
      << uint(0)
      << QString("deepin-defender")
      << QString("") << tr("You have not changed the password for %1 days, please change it regularly to improve system security.").arg(days)
      << action
      << inform
      << int(5000);
    QDBusMessage respone = QDBusConnection::sessionBus().call(m);
}

// 检查密码修改的天数
void LoginSafetySrvModel::checkPwdLastChangeDays()
{
    const qint64 deadline = getPwdChangeDeadline();
    const qint64 pwdLastChangedDays = daysToPasswdLastChanged();
    if (0 >= deadline) {
        return;
    }

    if (pwdLastChangedDays > deadline) {
        showNeedChangePwdSysNotify(pwdLastChangedDays);
    }
}
