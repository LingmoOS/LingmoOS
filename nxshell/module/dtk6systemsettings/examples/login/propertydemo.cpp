// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "propertydemo.h"

#include "dloginseat.h"
#include "dloginuser.h"

#include <qdebug.h>

#include <QMetaObject>
#include <QMetaProperty>

PropertyDemo::PropertyDemo(QObject *parent)
    : LoginDemo(parent)
    , m_manager(new DLoginManager)
{
    auto current = m_manager->currentSession();
    if (current) {
        m_currentSession = current.value();
    } else {
        m_currentSession = {};
    }
}

int PropertyDemo::run()
{
    const QMetaObject *mo = m_manager->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName()
                 << ", readable:" << mp.isReadable() << ", writable:" << mp.isWritable()
                 << ", value:" << mp.read(m_manager);
    }
    qDebug() << m_manager->scheduledShutdown();
    auto user = m_manager->findUserById(1000);
    mo = user.value()->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName()
                 << ", readable:" << mp.isReadable() << ", writable:" << mp.isWritable()
                 << ", value:" << mp.read(user.value().data());
    }
    qDebug() << user.value()->UID();
    auto session = m_manager->findSessionById(m_currentSession->id());
    mo = session.value()->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName()
                 << ", readable:" << mp.isReadable() << ", writable:" << mp.isWritable()
                 << ", value:" << mp.read(session.value().data());
    }
    qDebug() << session.value()->id();
    auto seat = m_manager->currentSeat();
    mo = seat.value()->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName()
                 << ", readable:" << mp.isReadable() << ", writable:" << mp.isWritable()
                 << ", value:" << mp.read(seat.value().data());
    }
    qDebug() << seat.value()->id();
    return 0;
}
