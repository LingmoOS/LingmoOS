/*
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIAINTERFACE_H
#define MEDIAINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class MediaInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Media1")

public:
    explicit MediaInterface(const QDBusObjectPath &path, QObject *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

public Q_SLOTS:
    void RegisterEndpoint(const QDBusObjectPath &path, const QVariantMap &properties, const QDBusMessage &msg);
    void UnregisterEndpoint(const QDBusObjectPath &path, const QDBusMessage &msg);

private:
    void runSetConfigurationAction(const QVariantMap &properties);
    void runSelectConfigurationAction(const QVariantMap &properties);
    void runClearConfigurationAction(const QVariantMap &properties);
    void runReleaseAction();

    QDBusObjectPath m_endpoint;
    QString m_service;
    QVariantMap m_properties;
};

#endif
