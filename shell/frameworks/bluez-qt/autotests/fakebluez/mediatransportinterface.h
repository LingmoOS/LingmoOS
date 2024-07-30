/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIATRANSPORTINTERFACE_H
#define MEDIATRANSPORTINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class MediaTransportObject : public QObject
{
public:
    explicit MediaTransportObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class MediaTransportInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.MediaTransport1")
    Q_PROPERTY(quint8 Codec READ codec)
    Q_PROPERTY(QByteArray Configuration READ configuration)
    Q_PROPERTY(QString State READ state)
    Q_PROPERTY(quint16 Volume READ volume)

public:
    explicit MediaTransportInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    quint8 codec() const;
    QByteArray configuration() const;
    QString state() const;
    quint16 volume() const;

public Q_SLOTS:
    void Acquire(const QDBusMessage &msg);
    void TryAcquire(const QDBusMessage &msg);
    void Release();
};

#endif
