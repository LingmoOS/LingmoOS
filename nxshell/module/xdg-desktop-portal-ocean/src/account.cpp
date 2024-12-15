// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "account.h"

#include <sys/types.h>
#include <unistd.h>

#include <QLoggingCategory>
#include <QVariantMap>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusConnection>

Q_LOGGING_CATEGORY(XdgDestkopOCEANAcount, "xdg-ocean-account")

AccountPortal::AccountPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDestkopOCEANAcount) << "account init";
}

uint AccountPortal::GetUserInformation(
    const QDBusObjectPath &handle, const QString &app_id, const QString &window, const QVariantMap &options, QVariantMap &results)
{
    qCDebug(XdgDestkopOCEANAcount) << "request for account";

    QString reason;

    if (options.contains(QStringLiteral("reason"))) {
        reason = options.value(QStringLiteral("reason")).toString();
    }

    QString idUserPath = QStringLiteral("/org/freedesktop/Accounts/User%1").arg(getuid());
    QDBusMessage UserMsg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.Accounts"),
                                                             idUserPath,
                                                             QStringLiteral("org.freedesktop.DBus.Properties"),
                                                             QStringLiteral("Get"));

    UserMsg.setArguments({QVariant::fromValue(QStringLiteral("org.freedesktop.Accounts.User")), QVariant::fromValue(QStringLiteral("IconFile"))});
    QDBusPendingReply<QDBusVariant> iconReply = QDBusConnection::systemBus().call(UserMsg);
    if (iconReply.isError())
        qCDebug(XdgDestkopOCEANAcount) << "setting failed";

    UserMsg.setArguments({QVariant::fromValue(QStringLiteral("org.freedesktop.Accounts.User")), QVariant::fromValue(QStringLiteral("RealName"))});
    QDBusPendingReply<QDBusVariant> realNameReply = QDBusConnection::systemBus().call(UserMsg);
    if (realNameReply.isError())
        qCDebug(XdgDestkopOCEANAcount) << "setting failed";

    UserMsg.setArguments({QVariant::fromValue(QStringLiteral("org.freedesktop.Accounts.User")), QVariant::fromValue(QStringLiteral("UserName"))});
    QDBusPendingReply<QDBusVariant> userNameReply = QDBusConnection::systemBus().call(UserMsg);
    if (userNameReply.isError())
        qCDebug(XdgDestkopOCEANAcount) << "setting failed";

    results.insert(QStringLiteral("id"), userNameReply.value().variant().toString());
    results.insert(QStringLiteral("name"), iconReply.value().variant().toString());
    results.insert(QStringLiteral("image"), realNameReply.value().variant().toString());

    return !results.isEmpty();
}
