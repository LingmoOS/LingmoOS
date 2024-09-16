// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamanager.h"

#include <QAtomicPointer>
#include <QDebug>

//静态成员变量初始化。
QMutex DataManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<DataManager> DataManager::m_instance = nullptr;//原子指针，默认初始化是0

//Q_GLOBAL_STATIC(DataManager, datamanager)

void DataManager::resetArchiveData()
{
    m_stArchiveData.reset();
}

ArchiveData &DataManager::archiveData()
{
    return m_stArchiveData;
}

DataManager::DataManager()
{

}

DataManager::DataManager(const DataManager &)
{

}

DataManager &DataManager::get_instance(void)
{

#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<DataManager>::isTestAndSetNative()) //运行时检测
        qInfo() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new DataManager);//第二次检测。
    }

    return *m_instance;

    //return datamanager();
}
