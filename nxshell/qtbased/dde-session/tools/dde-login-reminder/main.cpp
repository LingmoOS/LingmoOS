// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDateTime>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QNetworkInterface>

#include <unistd.h>

#include "utils.h"
#include "loginreminderinfo.h"


int start()
{
    qDebug() << "login reminder init";

    if (!Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "login-reminder", false).toBool())
        return 0;

    // 登录后展示横幅通知信息
    QDBusInterface userInter("org.deepin.dde.Accounts1", QString("/org/deepin/dde/Accounts1/User%1").arg(getuid()), "org.deepin.dde.Accounts1.User", QDBusConnection::systemBus());
    QDBusPendingReply<LoginReminderInfo> reply = userInter.call("GetReminderInfo");
    if (reply.isError()) {
        qWarning() << "failed to retrieve login reminder info, error: " << reply.error().message();
        return -1;
    }

    // 获取当前设备的ip
    auto getFirstIpAddress = [] {
        QList<QHostAddress> addressesList = QNetworkInterface::allAddresses();

        // 优先查找ipv4的地址
        auto ipV4Iter = std::find_if(addressesList.begin(), addressesList.end(), [ = ] (QHostAddress address) {
            return (address.protocol() == QAbstractSocket::IPv4Protocol)
                    && (address != QHostAddress::LocalHost)
                    && (address != QHostAddress::Broadcast)
                    && (address != QHostAddress::AnyIPv4)
                    && (address != QHostAddress::Any);
        });

        if (ipV4Iter != addressesList.end()) {
            return ipV4Iter->toString();
        }

        // 其次,寻找可用的ipv6地址
        auto ipV6Iter = std::find_if(addressesList.begin(), addressesList.end(), [ = ] (QHostAddress address) {
            return (address.protocol() == QAbstractSocket::IPv6Protocol)
                    && (address != QHostAddress::LocalHostIPv6)
                    && (address != QHostAddress::AnyIPv6)
                    && (address != QHostAddress::Any);
        });

        if (ipV6Iter != addressesList.end()) {
            return ipV6Iter->toString();
        }

        return QString();
    };

#define SECONDS_PER_DAY (60 * 60 * 24)
    const LoginReminderInfo &info = reply.value();

    const QString &title = QObject::tr("Login Reminder");
    const QString &currentLoginTime = info.CurrentLogin.Time.left(QString("yyyy-MM-dd hh:mm:ss").length());
    const QString &address = (info.CurrentLogin.Address == "0.0.0.0") ? getFirstIpAddress() : info.CurrentLogin.Address;
    QString body = QString("%1 %2 %3").arg(info.Username).arg(address).arg(currentLoginTime);
    int daysLeft = info.spent.LastChange + info.spent.Max - int(QDateTime::currentDateTime().toTime_t() / SECONDS_PER_DAY);
    if ((info.spent.Max != -1) && (info.spent.Warn != -1) && (info.spent.Warn > daysLeft)) {
        body += " " + QString(QObject::tr("Your password will expire in %1 days")).arg(daysLeft);
    }
    body += "\n" + QString(QObject::tr("%1 login failures since the last successful login")).arg(info.FailCountSinceLastLogin);

    const QString &lastLoginTime = info.LastLogin.Time.left(QString("yyyy-MM-dd hh:mm:ss").length());
    QString content;
    content += QString("<p>%1</p>").arg(info.Username);
    content += QString("<p>%1</p>").arg(address);
    content += "<p>" + QString(QObject::tr("Login time: %1")).arg(currentLoginTime) + "</p>";
    content += "<p>" + QString(QObject::tr("Last login: %1")).arg(lastLoginTime) + "</p>";
    content += "<p><b>" + QString("Your password will expire in %1 days").arg(daysLeft) + "</b></p>";
    content += "<br>";
    content += "<p>" + QString("%1 login failures since the last successful login").arg(info.FailCountSinceLastLogin) + "</p>";

    QDBusInterface *notifyInter = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert(QString("x-deepin-action-details"),QVariant(QString("/usr/bin/dde-hints-dialog, %1, %2").arg(title).arg(content)));
    QDBusPendingReply<uint> notifyIdReply = notifyInter->call("Notify", "dde-control-center", uint(0), "preferences-system", title, body, QStringList() << "details" << QObject::tr("Details"), hints, int(0));
    if (notifyIdReply.isError()) {
        qWarning() << "failed to call notification, error: " << notifyIdReply.error().message();
        return -1;
    }
    uint notifyId = notifyIdReply.value();

    // 5秒后自动关闭通知，避免在通知中心中显示
    sleep(5);

    QDBusPendingReply<> closeReply = notifyInter->call("CloseNotification", notifyId);
    if (closeReply.isError()) {
        qWarning() << "failed to close notification, error: " << closeReply.error().message();
        return -1;
    }

    qDebug() << "login reminder init finished";
    return 0;
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    registerLoginReminderInfoMetaType();

    return start();
}
