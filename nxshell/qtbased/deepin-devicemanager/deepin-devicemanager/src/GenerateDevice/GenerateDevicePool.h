// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERATEDEVICEPOOL_H
#define GENERATEDEVICEPOOL_H

#include <QObject>
#include <QThreadPool>
#include <QMutex>

/**
 * @brief The DeviceType enum
 */
enum DeviceType {
    DT_Null       = 0,
    DT_Audio      = 1,
    DT_Bios       = 2,
    DT_Bluetoorh  = 3,
    DT_Cdrom      = 4,
    DT_Computer   = 5,
    DT_Cpu        = 6,
    DT_Gpu        = 7,
    DT_Image      = 8,
    DT_Keyboard   = 9,
    DT_Memory     = 10,
    DT_Monitor    = 11,
    DT_Mouse      = 12,
    DT_Network    = 13,
    DT_OtherPCI   = 14,
    DT_Storage    = 15,
    DT_Power      = 16,
    DT_Print      = 17,
    DT_Others     = 18
};


/**
 * @brief The GenerateTask class
 * 线程池的任务类，生成设备的任务类
 */
class GenerateTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    GenerateTask(DeviceType deviceType);
    ~GenerateTask();
signals:
    void finished(const QStringList &lst);
protected:
    void run();
private:
    DeviceType           m_Type;
};



class GenerateDevicePool : public QThreadPool
{
    Q_OBJECT
public:
    GenerateDevicePool();

    /**
     * @brief generateDevice
     */
    void generateDevice();

private:
    /**
     * @brief initType
     */
    void initType();

private slots:
    /**
     * @brief slotFinished : end operation
     * @param lst
     */
    void slotFinished(const QStringList &lst);

private:
    QList<DeviceType>            m_TypeList;
    int                          m_FinishedGenerator;
};

#endif // GENERATEDEVICEPOOL_H
