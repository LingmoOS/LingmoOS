// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H

#include "types/arealist.h"

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>

class Service
    : public QObject,
      protected QDBusContext // QDBusContext非必须，可以不继承该类，这里用到是为了发送dbusError
{
    Q_OBJECT
    // property问题
    // 1, 没有发出PropertiesChange信号
    // 2, AgeChange信号需要在adaptor中有，目前是在adaptor中emit，在Service中emit实际也可以(不建议)
    // 3，Service不定义Age也没问题，但取不到该值，如果要用在Service中也定义
    // xml - property1, adaptor方案不需要实现
    //    Q_PROPERTY(bool Locked READ locked WRITE setLocked NOTIFY lockedChanged)
    //    Q_PROPERTY(QDBusObjectPath CurrentSessionPath READ currentSessionPath WRITE
    //    setCurrentSessionPath NOTIFY currentSessionPathChanged)
    Q_PROPERTY(quint32 Age READ Age WRITE SetAge NOTIFY AgeChanged)
    Q_PROPERTY(QString msg READ Msg WRITE SetMsg NOTIFY MsgChanged)

public:
    explicit Service(QObject *parent = 0);

    // xml - property2, adaptor方案不需要实现
    //    bool locked();
    //    void setLocked(bool value);
    //    QDBusObjectPath currentSessionPath();
    //    void setCurrentSessionPath(QDBusObjectPath value);
    quint32 Age();
    void SetAge(quint32 value);
    QString Msg();
    void SetMsg(QString value);

signals:
    // xml - signal
    void Tick();
    // xml - property3, adaptor方案不需要实现
    //    void lockedChanged(bool);
    //    void currentSessionPathChanged(QDBusObjectPath);
    void AgeChanged(const quint32 &age);
    void MsgChanged(const QString &msg);
    // 复杂类型信号demo，具体见arealist.h
    void AreaTestSig(const QString &id, AreaList aaa);

public slots:
    // xml - method
    QString Hello();
    void hidden_method();
    bool Register(const QString &id, QString &result2);
    // 复杂类型接口demo，具体见arealist.h
    bool SetArea(const QString &id, const AreaList area);

private:
    // xml - property, adaptor方案不需要实现
    //    bool m_locked;
    //    QDBusObjectPath m_currentSessionPath;
    quint32 m_age;
    QString m_msg;

protected:
    void connectNotify(const QMetaMethod &signal);
};

#endif // SERVICE_H
