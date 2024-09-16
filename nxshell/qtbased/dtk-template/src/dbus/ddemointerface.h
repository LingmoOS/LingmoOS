// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDEMOINTERFACE_H
#define DDEMOINTERFACE_H

#include "ddemotypes_p.h"
#include <QObject>
#include <DDBusInterface>
#include <QDBusPendingReply>

DDEMO_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DDBusInterface;

class DDemoInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDemoInterface(QObject *parent = nullptr);
    ~DDemoInterface() = default;

    // DBus的属性声明应放在这里。
    Q_PROPERTY(bool Docked READ Docked NOTIFY DockedChanged)

    bool Docked() const;

public Q_SLOTS:
    // DBus的方法调用应该放在这里。
    QDBusPendingReply<UserPathList_p> ListUsers();

Q_SIGNALS:
    // 使用DDBusInterface（m_interface)，并且其父对象指定为this，就可以自动使用属性Changed信号。
    // 需要注意的是，可能存在某些DBus接口属性改变不会发生Changed信号的情况，所以应提前做好测试。
    void DockedChanged(const bool value);

    // 此处是DBus接口声明的信号，在此进行声明，在构造函数进行连接。
    void UserNew(const uint uid, const QDBusObjectPath &path);

private:
    DDBusInterface *m_interface{nullptr};
};

DDEMO_END_NAMESPACE

#endif
