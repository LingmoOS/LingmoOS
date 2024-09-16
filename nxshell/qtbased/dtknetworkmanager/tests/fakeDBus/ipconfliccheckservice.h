// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IPCONFLICCHECKSERVICE_H
#define IPCONFLICCHECKSERVICE_H

#include <QDBusObjectPath>

class FakeIpConflictCheckService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeSystem.IPWatchD")
public:
    explicit FakeIpConflictCheckService(QObject *parent = nullptr);
    ~FakeIpConflictCheckService() override;

    bool m_requestIPConflictCheckTrigger{false};

public Q_SLOTS:

    Q_SCRIPTABLE QString RequestIPConflictCheck(QString, QString)
    {
        m_requestIPConflictCheckTrigger = true;
        return {};
    };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeSystem.IPWatchD"};
    const QString Path{"/com/deepin/FakeSystem/IPWatchD"};
};

#endif
