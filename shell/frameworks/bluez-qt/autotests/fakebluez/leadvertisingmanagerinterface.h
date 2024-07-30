/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LEADVERTISINGMANAGERINTERFACE_H
#define LEADVERTISINGMANAGERINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class LEAdvertisingManagerInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.LEAdvertisingManager1")

public:
    explicit LEAdvertisingManagerInterface(const QDBusObjectPath &path, QObject *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

public Q_SLOTS:
    void RegisterAdvertisement(const QDBusObjectPath &path, const QVariantMap &options, const QDBusMessage &msg);
    void UnregisterAdvertisement(const QDBusObjectPath &path, const QDBusMessage &msg);

private:
    void runReleaseAction();

    QDBusObjectPath m_advertisement;
    QString m_service;
};

#endif
