// Copyright (C) 2022 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHORITYWRAPPER_H
#define AUTHORITYWRAPPER_H

#include "defenderauthority.h"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

class DefenderAuthority;

class DefenderAuthorityWrapper : public QObject
{
    Q_OBJECT

public:
    explicit DefenderAuthorityWrapper(QObject *parent);
    ~DefenderAuthorityWrapper();

    void initConnection();

Q_SIGNALS:
    // 转发信号
    void notifyAuthStarted();
    void notifyAuthFinished();
    void notifyAuthResult(const QString &moduleName, int id, bool result);
    void onAuthRequest(const QString &moduleName, int id);

    void onModulesActived();

private:
    QThread m_authThread;
    DefenderAuthority *m_authority;
};

#endif // AUTHORITYWRAPPER_H
