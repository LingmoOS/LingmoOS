// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARGLOBALENV_H
#define CALENDARGLOBALENV_H

#include <QVariant>

/**
 * @brief The CalendarGlobalEnv class       日历全局环境变量类，用来保存一些全局唯一的变量
 */
class CalendarGlobalEnv
{
public:
    /**
     * @brief getGlobalEnv
     * @return
     */
    static CalendarGlobalEnv *getGlobalEnv();
    CalendarGlobalEnv *operator->() const;
    /**
     * @brief registerKey           注册关键字
     * @param key                   关键字
     * @param variant               值
     * @return                      是否成功，若key已经被注册返回false,否则返回true
     */
    bool registerKey(const QString &key, const QVariant &variant);
    /**
     * @brief reviseValue           修改值
     * @param key                   关键字
     * @param variant               值
     * @return                      是否成功，若key没有被注册则返回false，否则返回true
     */
    bool reviseValue(const QString &key, const QVariant &variant);
    /**
     * @brief removeKey             移除注册的关键字
     * @param key                   关键字
     * @return                      若key没有被注册则返回false，否则返回true
     */
    bool removeKey(const QString &key);
    /**
     * @brief getValueByKey         根据关键字获取对应的值
     * @param key                   关键字
     * @param variant               获取到的值
     * @return                      若key没有被注册则返回false，否则返回true
     */
    bool getValueByKey(const QString &key, QVariant &variant);
private:
    CalendarGlobalEnv();
    CalendarGlobalEnv(CalendarGlobalEnv &) = delete;
    CalendarGlobalEnv &operator=(const CalendarGlobalEnv &) = delete;
private:
    QMap<QString, QVariant>     m_GlobalEnv;
};

#endif // CALENDARGLOBALENV_H
