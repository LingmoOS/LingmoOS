// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

#include <QDebug>
#include <QMetaMethod>
#include <QThread>
#include <QTimer>

Service::Service(QObject *parent)
    : QObject(parent)
    //    , m_currentSessionPath(QDBusObjectPath("/tt"))
    , m_age(11)
//    , m_msg("ss&22") // error
{
    // 测试发送信号
    QTimer *timer = new QTimer(this);
    timer->setInterval(5000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &Service::Tick);
    //    timer->start();

    registerAreaListMetaType();
}

QString Service::Hello()
{
    //    QThread::sleep(10);
    qInfo() << "Service::Hello called."
            << "ThreadID:" << QThread::currentThreadId();

    emit Tick();
    return "World";
}

void Service::hidden_method()
{
    qDebug() << __FUNCTION__;
}

bool Service::Register(const QString &id, QString &result2)
{
    qInfo() << "Service::Register called. id:" << id;

    // 返回一个dbus error类型
    // 1, Service继承QDBusContext
    // 2，调用sendErrorReply
    // 此例子中，调用方不会拿到结果true，而是接到一个QDBusError
    if (id == "error") {
        // dbus的官方错误类型,如QDBusError::Failed
        // sendErrorReply(QDBusError::Failed, "The method call 'Register()' is not supported");
        // 自定义错误类型
        // 1, 自定义类型第一个参数是name，格式需要以"."分割的字符串，比如"org.aaa"合法，"aaa"不合法
        // 2, 参考dbus官方错误类型, errorname都是"${dbusname}.${自定义错误名称}"的命名
        sendErrorReply("com.deepin.dbusdemo.RegisterNotSupported",
                       "The method call 'Register()' is not supported");
    }
    result2 = "reply2";
    return true;
}

bool Service::SetArea(const QString &id, const AreaList area)
{
    qInfo() << "SetArea, id:" << id << ", area size:" << area.size();
    for (auto v : area) {
        qInfo() << "area.x1:" << v.x1;
    }
    emit AreaTestSig("test-reply", area);
    return true;
}

// bool Service::locked()
//{
//     qInfo() << "Service::locked";
//     return m_locked;
// }

// void Service::setLocked(bool value)
//{
//     qInfo() << "Service::setLocked value:" << value;
//     m_locked = value;
// }

// QDBusObjectPath Service::currentSessionPath()
//{
//     qInfo() << "Service::currentSessionPath";
//     return m_currentSessionPath;
// }

// void Service::setCurrentSessionPath(QDBusObjectPath value)
//{
//     qInfo() << "Service::setCurrentSessionPath value:" << value.path();
//     m_currentSessionPath = value;
// }

quint32 Service::Age()
{
    qInfo() << "Service::age";
    return m_age;
}

void Service::SetAge(quint32 value)
{
    qInfo() << "Service::setAge value:" << value;
    m_age = value;
    //    emit AgeChanged(m_age); // 这里发送也会生效，但是建议到adaptor中实现emit
}

QString Service::Msg()
{
    qInfo() << "Service::Msg";
    return m_msg;
}

void Service::SetMsg(QString value)
{
    qInfo() << "Service::SetMsg value:" << value;
    m_msg = value;
}

void Service::connectNotify(const QMetaMethod &signal)
{
    qInfo() << "signal.methodType():" << signal.methodType();
    qInfo() << "signal.methodSignature():" << signal.methodSignature();
    //    QObject::connectNotify(signal);
}
