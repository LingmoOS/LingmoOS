// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <qobject.h>

class QFileSystemWatcher;
DLOGIN_BEGIN_NAMESPACE
class DLoginSession;
class Login1SessionInterface;
class StartManagerInterface;
class SessionManagerInterface;

class DLoginSessionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DLoginSessionPrivate(DLoginSession *parent = nullptr)
        : q_ptr(parent)
    {
    }

    bool enableAutostartWatch();
    QString getUserAutostartDir();
    QStringList getSystemAutostartDirs();
    QStringList getAutostartDirs();
    bool judgeAutostart(const QString &fullPath);
    QStringList getAutostartApps(const QString &dir);

private:
    Login1SessionInterface *m_inter;
    StartManagerInterface *m_startManagerInter;
    SessionManagerInterface *m_sessionManagerInter;
    DLoginSession *q_ptr;
    Q_DECLARE_PUBLIC(DLoginSession)
};

DLOGIN_END_NAMESPACE
