// SPDX-FileCopyrightText: 2018 - 2024 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dbusproxybase.hpp"

#include <QMap>
#include <QVector>
#include <functional>
#include <QSharedPointer>


class DBusProxyManager {
protected:
    using creaetDbusProxy = std::function<DBusProxyBase* ()>;

public:
    DBusProxyManager(QDBusConnection::BusType busType)
    : m_busType(busType)
    {

    }
    
    virtual void init() = 0;

    void createDBusProxy(){
        for(auto creatFunc : m_dbusProxyFactory){
            creatFunc();
        }
    }

protected:
    QVector<creaetDbusProxy> m_dbusProxyFactory;
    QDBusConnection::BusType m_busType;
};
