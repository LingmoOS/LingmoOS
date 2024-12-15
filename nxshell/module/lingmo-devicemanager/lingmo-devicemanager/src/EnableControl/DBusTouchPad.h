// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSTOUCHPAD_H
#define DBUSTOUCHPAD_H

#include <QObject>

class QDBusInterface;
class DBusTouchPad : public QObject
{
    Q_OBJECT
public:
    static DBusTouchPad *instance()
    {
        if (!sInstance) {
            sInstance = new DBusTouchPad;
        }
        return sInstance;
    }
    /**
    * @brief 是否存在触摸板
    * @return 布尔值，true:存在；false:不存在
    */
    bool isExists();
    /**
    * @brief 设置触摸板状态
    * @bool 布尔值，设置触摸板状态，true:使能状态；false:去使能状态
    */
    void setEnable(bool);
    /**
    * @brief 获取触摸板状态
    * @return 布尔值，返回触摸板当前状态，true:使能状态；false:去使能状态
    */
    bool getEnable();
protected:
    DBusTouchPad();
    ~DBusTouchPad();
private:
    static DBusTouchPad    *sInstance;
    QDBusInterface     *m_dbusTouchPad;   //mouse dbus对象
};

#endif // DBUSTOUCHPAD_H
