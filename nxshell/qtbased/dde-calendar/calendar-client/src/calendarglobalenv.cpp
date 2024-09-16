// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarglobalenv.h"

CalendarGlobalEnv *CalendarGlobalEnv::getGlobalEnv()
{
    static CalendarGlobalEnv globalEnv;
    return &globalEnv;
}

CalendarGlobalEnv *CalendarGlobalEnv::operator->() const
{
    return getGlobalEnv();
}

//注册关键字
bool CalendarGlobalEnv::registerKey(const QString &key, const QVariant &variant)
{
    //如果不包含则添加
    if (!m_GlobalEnv.contains(key)) {
        m_GlobalEnv[key] = variant;
        return true;
    }
    return false;
}

//修改值
bool CalendarGlobalEnv::reviseValue(const QString &key, const QVariant &variant)
{
    //如果包含，则修改
    if (m_GlobalEnv.contains(key)) {
        m_GlobalEnv[key] = variant;
        return true;
    }
    return false;
}

//移除注册的关键字
bool CalendarGlobalEnv::removeKey(const QString &key)
{
    //如果包含则移除
    if (m_GlobalEnv.contains(key)) {
        m_GlobalEnv.remove(key);
        return true;
    }
    return false;
}

//根据关键字获取对应的值
bool CalendarGlobalEnv::getValueByKey(const QString &key, QVariant &variant)
{
    if (m_GlobalEnv.contains(key)) {
        variant = m_GlobalEnv.value(key);
        return true;
    }
    return false;
}

CalendarGlobalEnv::CalendarGlobalEnv()
    : m_GlobalEnv{}
{

}
