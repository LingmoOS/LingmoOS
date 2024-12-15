// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "secret.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopOCEANSecret, "xdg-ocean-secret")

SecretPortal::SecretPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDesktopOCEANSecret) << "secret init";
}

uint SecretPortal::RetrieveSecret(const QDBusObjectPath &handle,
                                  const QString &app_id,
                                  const QDBusUnixFileDescriptor &fd,
                                  const QVariantMap &options,
                                  QVariantMap &result)
{
    qCDebug(XdgDesktopOCEANSecret) << "RetrieveSecret";
    return 1;
}
