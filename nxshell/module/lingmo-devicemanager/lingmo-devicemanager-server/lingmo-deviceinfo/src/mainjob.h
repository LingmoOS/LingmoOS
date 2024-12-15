// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>

class DeviceInterface;
class ThreadPool;
class DetectThread;
class MainJob : public QObject
{
    Q_OBJECT

public:
    explicit MainJob(const char *name, QObject *parent = nullptr);

    /**
     * @brief serverIsRunning
     * @return
     */
    static bool serverIsRunning();

    /**
     * @brief clientIsRunning
     * @return
     */
    static bool clientIsRunning();

    /**
     * @brief setWorkingFlag 设置工作状态
     */
    void setWorkingFlag(bool flag);

private slots:
    /**
     * @brief slotUsbChanged
     */
    void slotUsbChanged();
    /**
     * @brief slotUsbChanged
     * @param usbchanged
     */
    void slotDriverControl(bool success);
    /**
     * @brief slotWakeupHandle
     */
    void slotWakeupHandle(bool);
private:
    /**
     * @brief sqlCopytoKernel
     */
    void sqlCopytoKernel();
    /**
     * @brief initDriverRepoSource : 初始化驱动仓库
     * @return : 无
     */
    void initDriverRepoSource();
    /**
     * @brief updateAllDevice
     */
    void updateAllDevice();
    /**
     * @brief executeClientInstruction
     * @param instructions
     */
    void executeClientInstruction(const QString &instructions);

private:
    ThreadPool            *m_pool = nullptr;                  //<! 生成文件的线程池
    bool                   m_firstUpdate;                      //<! 是否是第一次更新
    DeviceInterface       *m_deviceInterface = nullptr;        //<! 设备信息
    DetectThread          *mp_DetectThread = nullptr;         //<! 检测usb的线程
};

#endif // MAINJOB_H
