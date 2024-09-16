// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEQTDBUS_H
#define SERVICEQTDBUS_H

#include "servicebase.h"

class QLibrary;

class ServiceQtDBus : public ServiceBase
{
    Q_OBJECT
public:
    explicit ServiceQtDBus(QObject *parent = nullptr);

    QDBusConnection qDbusConnection();

    virtual bool registerService() override;
    virtual bool unregisterService() override;

protected:
    virtual void initThread() override;

private:
    bool libFuncCall(const QString &funcName, bool isRegister);

private:
    QLibrary *m_library;
};

#endif // SERVICEQTDBUS_H
