/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTMANAGERINTERFACE_H
#define GATTMANAGERINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class GattManagerInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattManager1")

public:
    explicit GattManagerInterface(const QDBusObjectPath &path, QObject *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

public Q_SLOTS:
    void RegisterApplication(const QDBusObjectPath &path, const QVariantMap &options, const QDBusMessage &msg);
    void UnregisterApplication(const QDBusObjectPath &path, const QDBusMessage &msg);

private:
    void runGetObjectsAction();
    void runReadCharcAction(const QVariantMap &properties);
    void runWriteCharcAction(const QVariantMap &properties);

    QDBusObjectPath m_application;
    QDBusObjectPath m_characteristic;
    QString m_service;
    QVariantMap m_properties;
};

#endif
