// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "secret.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopDDESecret, "xdg-dde-secret")

SecretPortal::SecretPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDesktopDDESecret) << "secret init";
}

uint SecretPortal::RetrieveSecret(const QDBusObjectPath &handle,
                                  const QString &app_id,
                                  const QDBusUnixFileDescriptor &fd,
                                  const QVariantMap &options,
                                  QVariantMap &result)
{
    qCDebug(XdgDesktopDDESecret) << "RetrieveSecret";
    return 1;
}
