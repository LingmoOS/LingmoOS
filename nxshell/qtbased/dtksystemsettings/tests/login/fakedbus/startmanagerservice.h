// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dtklogin_global.h"

#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE

class StartManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.StartManager")
public:
    StartManagerService(QObject *parent = nullptr);
    ~StartManagerService() override;

public Q_SLOTS:
    Q_SCRIPTABLE bool AddAutostart(const QString &fileName);
    Q_SCRIPTABLE bool RemoveAutostart(const QString &fileName);

public:
    QString m_fileName;
    bool m_success;

private:
    bool registerService();
    void unRegisterService();
};

DLOGIN_END_NAMESPACE
