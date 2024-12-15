// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "commonstruct.h"

#include <QObject>
#include <QMutex>

class DataManager
{
public:
    //DataManager();//防止构造函数
    /**
     * @brief get_instance  获取单例实例对象
     * @return
     */
    static DataManager &get_instance(void);

    /**
     * @brief resetArchiveData  重置数据
     */
    void resetArchiveData();

    /**
     * @brief archiveData   获取压缩包数据
     * @return  数据
     */
    ArchiveData &archiveData();

private:
    DataManager();//防止构造函数
    DataManager(const DataManager &);//防止拷贝构造函数

    ArchiveData m_stArchiveData;        // 存储当前操作的压缩包数据

    static QMutex m_mutex;//实例互斥锁。
    static QAtomicPointer<DataManager> m_instance;/*!<使用原子指针,默认初始化为0。*/
};



#endif // DATAMANAGER_H
