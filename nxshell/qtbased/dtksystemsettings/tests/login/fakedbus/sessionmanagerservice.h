// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dtklogin_global.h"

#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE

class SessionManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.SessionManager")
public:
    SessionManagerService(QObject *parent = nullptr);
    ~SessionManagerService() override;

    Q_PROPERTY(bool Locked READ Locked NOTIFY LockedChanged);

    bool Locked();

Q_SIGNALS:
    Q_SCRIPTABLE void LockedChanged(const bool Locked);

public Q_SLOTS:
    Q_SCRIPTABLE void SetLocked(const bool Locked);

public:
    bool m_locked;

private:
    bool registerService();
    void unRegisterService();
};

DLOGIN_END_NAMESPACE
