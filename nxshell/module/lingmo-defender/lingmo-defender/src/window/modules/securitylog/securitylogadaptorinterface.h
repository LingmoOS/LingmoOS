// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYLOGADAPTORINTERFACE_H
#define SECURITYLOGADAPTORINTERFACE_H

class QSqlDatabase;
class QString;

class SecurityLogAdaptorInterface
{
public:
    virtual QSqlDatabase GetDatabase() = 0;
    virtual bool DeleteSecurityLog(int, int) = 0;
    virtual void AddSecurityLog(int nType, const QString &sInfo) = 0;
    virtual ~SecurityLogAdaptorInterface() {}
};

#endif // SECURITYLOGADAPTORINTERFACE_H
