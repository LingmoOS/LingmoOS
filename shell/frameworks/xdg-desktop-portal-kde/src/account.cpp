/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#include "account.h"
#include "account_debug.h"
#include "userinfodialog.h"
#include "utils.h"

AccountPortal::AccountPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

uint AccountPortal::GetUserInformation(const QDBusObjectPath &handle,
                                       const QString &app_id,
                                       const QString &parent_window,
                                       const QVariantMap &options,
                                       QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeAccount) << "GetUserInformation called with parameters:";
    qCDebug(XdgDesktopPortalKdeAccount) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeAccount) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeAccount) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeAccount) << "    options: " << options;

    QString reason;

    if (options.contains(QStringLiteral("reason"))) {
        reason = options.value(QStringLiteral("reason")).toString();
    }

    UserInfoDialog *userInfoDialog = new UserInfoDialog(reason);
    Utils::setParentWindow(userInfoDialog->windowHandle(), parent_window);

    int result = userInfoDialog->exec();

    if (result) {
        results.insert(QStringLiteral("id"), userInfoDialog->id());
        results.insert(QStringLiteral("name"), userInfoDialog->name());
        results.insert(QStringLiteral("image"), userInfoDialog->image());
    }

    userInfoDialog->deleteLater();

    return !result;
}
