// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QTDBUSHOOK_H
#define QTDBUSHOOK_H

#include "servicebase.h"

typedef QMap<QString, ServiceBase *> ServiceObjectMap;

class QTDbusHook
{
public:
    explicit QTDbusHook();

    bool getServiceObject(
            QString name, QString path, ServiceBase **service, bool &isSubPath, QString &realPath);

    bool setServiceObject(ServiceBase *obj);

    static QTDbusHook *instance();

private:
    ServiceObjectMap m_serviceMap;
};

#endif // QTDBUSHOOK_H
