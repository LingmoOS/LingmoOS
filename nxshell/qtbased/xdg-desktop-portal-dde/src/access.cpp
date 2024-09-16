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

Q_LOGGING_CATEGORY(XdgDestkopDDEAccess, "xdg-dde-access")

AccessPortal::AccessPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDestkopDDEAccess) << "access init";
}

uint AccessPortal::AccessDialog(
        const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QString &title,
        const QString &subtitle, const QString &body, const QVariantMap &options, QVariantMap &results)
{
    qCDebug(XdgDestkopDDEAccess) << "request for access dialog";

    QMessageBox access_dialog;

    if (options.contains(QStringLiteral("modal"))) {
        access_dialog.setModal(options.value(QStringLiteral("modal")).toBool());
    }

    QPushButton *rejectButton = nullptr;
    if (options.contains(QStringLiteral("deny_label"))) {
        rejectButton = access_dialog.addButton(options.value(QStringLiteral("deny_label")).toString(), QMessageBox::RejectRole);
    }


    QPushButton *allowButton = nullptr;
    if (options.contains(QStringLiteral("grant_label"))) {
        allowButton = access_dialog.addButton(options.value(QStringLiteral("grant_label")).toString(), QMessageBox::AcceptRole);
    }

    access_dialog.setWindowTitle(title);
    access_dialog.setText(body);
    access_dialog.exec();

    uint respnse = 2;
    if (access_dialog.clickedButton() == (QAbstractButton*)rejectButton) {
        respnse = 0;
    } else if (access_dialog.clickedButton() == (QAbstractButton*)allowButton) {
        respnse = 1;
    }

    return respnse;
}
