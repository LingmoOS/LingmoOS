// Copyright (C) 2024 Wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "access.h"

#include <sys/types.h>
#include <unistd.h>

#include <QLoggingCategory>
#include <QVariantMap>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QMessageBox>
#include "accessdialog.h"

Q_LOGGING_CATEGORY(XdgDestkopOCEANAccess, "xdg-ocean-access")

AccessPortal::AccessPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDestkopOCEANAccess) << "access init";
}

using AccessDialogClass = AccessDialog;

uint AccessPortal::AccessDialog(
        const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QString &title,
        const QString &subtitle, const QString &body, const QVariantMap &options, QVariantMap &results)
{
    qCDebug(XdgDestkopOCEANAccess) << "request for access dialog";
    AccessDialogClass dialog(app_id,parent_window,title,subtitle,body,options);
    return dialog.exec();
}
