// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OBEXTRANSFERSERVICE_H
#define OBEXTRANSFERSERVICE_H

#include <QDBusObjectPath>
#include "dbluetoothutils.h"

class FakeObexTransferService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Transfer1")
public:
    explicit FakeObexTransferService(QObject *parent = nullptr)
        : QObject(parent)
    {
        registerService();
    }

    ~FakeObexTransferService() { unregisterService(); }

    Q_PROPERTY(QString Status READ status)
    Q_PROPERTY(QDBusObjectPath Session READ session)
    Q_PROPERTY(QString Name READ name)
    Q_PROPERTY(quint64 Size READ size)
    Q_PROPERTY(quint64 Transferred READ transferred)
    Q_PROPERTY(QString Filename READ filename)
    Q_PROPERTY(QString Type READ type)

    QString m_status{"queued"};
    QDBusObjectPath m_session{"/org/bluez/obex/client/session0"};
    QString m_name{"transfer1"};
    quint64 m_size{1024};
    quint64 m_transferred{512};
    QString m_filename{"1.txt"};
    QString m_type{"text"};

    QString status() const { return m_status; };
    QDBusObjectPath session() const { return m_session; };
    QString type() const { return m_type; };
    QString name() const { return m_name; };
    quint64 size() const { return m_size; };
    quint64 transferred() const { return m_transferred; };
    QString filename() const { return m_filename; };

    bool m_cancel{false};
    bool m_suspend{false};
    bool m_resume{false};

public Q_SLOTS:
    void Cancel() { m_cancel = true; }
    void Suspend() { m_suspend = true; }
    void Resume() { m_resume = true; }

private:
    void registerService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.registerService(Dtk::Bluetooth::FakeBlueZObexService)) {
            QString errorMsg = bus.lastError().message();
            if (errorMsg.isEmpty())
                errorMsg = "maybe it's running";
            qWarning() << QString("Can't register the %1 service, %2.").arg(Dtk::Bluetooth::FakeBlueZObexService).arg(errorMsg);
        }
        if (!bus.registerObject("/org/bluez/obex/client/session0/transfer0", this, QDBusConnection::ExportAllContents)) {
            qWarning() << QString("Can't register %1 the D-Bus object.").arg("/org/bluez/obex/client/session0/transfer0");
        }
    }
    void unregisterService()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.unregisterObject("/org/bluez/obex/client/session0/transfer0");
        bus.unregisterService(Dtk::Bluetooth::FakeBlueZObexService);
    }
};

#endif
