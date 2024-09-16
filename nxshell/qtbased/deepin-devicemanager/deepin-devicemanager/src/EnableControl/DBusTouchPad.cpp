// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusTouchPad.h"
#include "MacroDefinition.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QDBusInterface>

#ifdef OS_BUILD_V23
const QString Service = "org.deepin.dde.InputDevices1";
const QString Path = "/org/deepin/dde/InputDevice1/TouchPad";
const QString Interface = "org.deepin.dde.InputDevice1.TouchPad";
#else
const QString Service = "com.deepin.daemon.InputDevices";
const QString Path = "/com/deepin/daemon/InputDevice/TouchPad";
const QString Interface = "com.deepin.daemon.InputDevice.TouchPad";
#endif

DBusTouchPad *DBusTouchPad::sInstance = nullptr;
DBusTouchPad::DBusTouchPad()
    : QObject(nullptr)
    , m_dbusTouchPad(new QDBusInterface(Service, Path, Interface, QDBusConnection::sessionBus(), this))
{
}

DBusTouchPad::~DBusTouchPad()
{
    DELETE_PTR(m_dbusTouchPad);
}
/**
* @brief 是否存在触摸板
* @return 布尔值，true:存在；false:不存在
*/
bool DBusTouchPad::isExists()
{
    if (m_dbusTouchPad->isValid()) {
        return m_dbusTouchPad->property("Exist").toBool();
    }
    return false;
}

/**
* @brief 设置触摸板状态
* @bool 布尔值，设置触摸板状态，true:使能状态；false:去使能状态
*/
void DBusTouchPad::setEnable(bool state)
{
    if (m_dbusTouchPad->isValid()) {
        m_dbusTouchPad->setProperty("TPadEnable", state);
    }
}

/**
* @brief 获取触摸板状态
* @return 布尔值，返回触摸板当前状态，true:使能状态；false:去使能状态
*/
bool DBusTouchPad::getEnable()
{
    if (m_dbusTouchPad->isValid()) {
        return m_dbusTouchPad->property("TPadEnable").toBool();
    }
    return false;
}
