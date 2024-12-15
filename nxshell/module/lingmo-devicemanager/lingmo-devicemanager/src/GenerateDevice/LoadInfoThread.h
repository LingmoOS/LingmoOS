// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADINFOTHREAD_H
#define LOADINFOTHREAD_H

#include <QObject>
#include <QThread>
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"

class LoadInfoThread : public QThread
{
    Q_OBJECT
public:
    LoadInfoThread();
    ~LoadInfoThread();

    /**
     * @brief setFramework：设置架构
     * @param arch:架构
     */
    void setFramework(const QString &arch);

signals:
    void finished(const QString &message);
    void finishedReadFilePool();

protected:
    void run() override;

private slots:
    /**
     * @brief slotFinishedReadFilePool
     * @param info
     */
    void slotFinishedReadFilePool(const QString &info);

private:
    GetInfoPool mp_ReadFilePool;
    GenerateDevicePool mp_GenerateDevicePool;
    bool            m_Running;                      //<!  标识是否正在运行
    bool            m_FinishedReadFilePool;         //<!  标识生成读文件的线程池是否结束
    bool            m_Start;                        //<!  是否为启动

};

#endif // LOADINFOTHREAD_H
