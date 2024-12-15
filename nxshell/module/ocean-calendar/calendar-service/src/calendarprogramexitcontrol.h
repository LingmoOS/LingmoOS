// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARPROGRAMEXITCONTROL_H
#define CALENDARPROGRAMEXITCONTROL_H

#include <QReadWriteLock>

/**
 * @brief The CalendarProgramExitControl class
 * 后端程序退出控制
 * 采用计数的方式来判断程序是否退出
 * 在接口入口处添加计数，接口处理结束后减少计数，若计数为0则程序退出
 */
class CalendarProgramExitControl
{
public:
    static CalendarProgramExitControl *getProgramExitControl();
    CalendarProgramExitControl *operator->() const;

    /**
     * @brief addExc    接口入口处添加计数，跟reduce配对使用
     */
    void addExc();

    /**
     * @brief reduce      接口处理结束后使用该函数，用来减少计数，跟addExc配对使用
     */
    void reduce();
    static bool getClientIsOpen();
    static void setClientIsOpen(bool clientIsOpen);

private:
    CalendarProgramExitControl();
    void exit();
private:
    static bool m_clientIsOpen;
    int   m_excNum = 0;
    QReadWriteLock readWriteLock;
};

class DServiceExitControl
{
public:
    DServiceExitControl()
    {
        CalendarProgramExitControl::getProgramExitControl()->addExc();
    }
    ~DServiceExitControl()
    {
        CalendarProgramExitControl::getProgramExitControl()->reduce();
    }
    void setClientIsOpen(bool isOpen)
    {
        CalendarProgramExitControl::setClientIsOpen(isOpen);
    }
};

#endif // CALENDARPROGRAMEXITCONTROL_H
