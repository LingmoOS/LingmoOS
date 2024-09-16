// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYLOGADAPTORIMPL_H
#define SECURITYLOGADAPTORIMPL_H

#include "securitylogadaptorinterface.h"
#include <QObject>
#include <QSqlDatabase>

class ComDeepinDefenderMonitorNetFlowInterface;

class SecurityLogAdaptorImpl : public QObject
    , public SecurityLogAdaptorInterface
{
    Q_OBJECT
public:
    explicit SecurityLogAdaptorImpl(QObject *parent = nullptr);
    virtual QSqlDatabase GetDatabase();
    virtual bool DeleteSecurityLog(int, int);
    virtual void AddSecurityLog(int nType, const QString &sInfo);
    virtual ~SecurityLogAdaptorImpl() {}

private:
    ComDeepinDefenderMonitorNetFlowInterface *m_monitorInterface;
    QSqlDatabase m_sqlDb;

private:
    // 禁用拷贝构造与赋值
    SecurityLogAdaptorImpl(const SecurityLogAdaptorImpl &);
    SecurityLogAdaptorImpl &operator=(const SecurityLogAdaptorImpl &);
};

#endif // SECURITYLOGADAPTORIMPL_H
