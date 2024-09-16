// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDBusVirtualObject>

#include "com_deepin_daemon_SoundEffect.h"

class SoundEffect : public QDBusVirtualObject {
public:
	SoundEffect(QObject *parent = nullptr) :QDBusVirtualObject(parent)
    {
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.daemon.SoundEffect").registerService("com.test.daemon.SoundEffect");
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.daemon.SoundEffect").registerVirtualObject("/com/test/daemon/SoundEffect", this);
        m_soundEffect = new com::deepin::daemon::SoundEffect("com.deepin.daemon.SoundEffect", "/com/deepin/daemon/SoundEffect", QDBusConnection::sessionBus(), this);
        signalMonitor();
    }
    virtual	~SoundEffect(){}
    virtual bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
    {
        qInfo() << "asdsad";
        //     inline QDBusPendingReply<QString> GetSoundFile(const QString &name)
        if (message.member() == "GetSoundFile") {
            qInfo() << "GetSoundFile:" << message.arguments().at(0).toString();
            QDBusReply<QString> reply = m_soundEffect->GetSoundFile(message.arguments().at(0).toString());
            qInfo() << "reply:" << reply.value();
            connection.send(message.createReply(reply.value()));
        }
        return true;
    }
    virtual QString	introspect(const QString &path) const
    {
        return "";
    }
    void signalMonitor()
    {
        connect(m_soundEffect, &com::deepin::daemon::SoundEffect::EnabledChanged, this, [](bool ret){
            qInfo() << "enable:" << ret;
        });
    }

private:
    com::deepin::daemon::SoundEffect *m_soundEffect;
};


