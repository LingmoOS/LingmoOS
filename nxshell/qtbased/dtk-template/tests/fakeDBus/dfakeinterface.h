// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DFAKEINTERFACE_H
#define DFAKEINTERFACE_H

#include "ddemotypes_p.h"
#include <QObject>
#include <QDBusObjectPath>

class DFakeInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.fakelogin.Manager")
public:
    explicit DFakeInterface(QObject *parent = nullptr);
    virtual ~DFakeInterface();

    Q_PROPERTY(bool Docked READ Docked WRITE setDocked NOTIFY DockedChanged)
    bool Docked() const;
    void setDocked(bool docked = true);

public slots:
    Dtk::Demo::UserPathList_p ListUsers();
    void setListUsers(const Dtk::Demo::UserPathList_p &list);  // addUser ?

signals:
    void DockedChanged(const bool value);
    void UserNew(uint id, const QDBusObjectPath &path);

private:
    bool registerService();
    void unRegisterService();

    Dtk::Demo::UserPathList_p m_users{};
    bool m_docked{false};
};

#endif  // DFAKEINTERFACE_H
