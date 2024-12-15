// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATADEALSINGLETON_H
#define DATADEALSINGLETON_H
#include <QMutex>
#include <QReadWriteLock>
#include <QObject>
#include <QTimer>

class CPUInfoModel;

class DataDealSingleton : public QObject
{
    Q_OBJECT
public:
    static DataDealSingleton &getInstance();

    //公开的接口
    //!
    //! \brief readCpuPer
    //! \param cpuPer
    //! \return
    //!
    bool readCpuPer(qreal &cpuPer);

    //!
    //! \brief readMemInfo
    //! \param memUsage
    //! \param memTotal
    //! \param memPercent
    //! \param swapUsage
    //! \param swapTotal
    //! \param swapPercent
    //! \return
    //!
    bool readMemInfo(QString &memUsage, QString &memTotal, QString &memPercent, QString &swapUsage, QString &swapTotal, QString &swapPercent);

    //!
    //! \brief readNetInfo
    //! \param netReceive
    //! \param netTotalReceive
    //! \param netSend
    //! \param totalSend
    //! \return
    //!
    bool readNetInfo(QString &netReceive, QString &netTotalReceive, QString &netSend, QString &totalSend);

    //!
    //! \brief readDiskInfo
    //! \param diskRead
    //! \param diskTotalSize
    //! \param diskWrite
    //! \param diskAvailable
    //! \return
    //!
    bool readDiskInfo(QString &diskRead, QString &diskTotalSize, QString &diskWrite, QString &diskAvailable);

    //!
    //! \brief sendJumpWidgetMessage 发送跳转命令消息
    //! \param dbusMessage 消息名称字符串
    //! \return 返回dbus信号是否发送成功
    //!
    bool sendJumpWidgetMessage(const QString &dbusMessage);

signals:
    void sigDataUpdate();

private:
    DataDealSingleton(QObject *parent = nullptr);
    ~DataDealSingleton();
    DataDealSingleton(const DataDealSingleton &) = delete;
    DataDealSingleton &operator=(const DataDealSingleton &) = delete;
    bool launchMainProcessByAM() const;

    QReadWriteLock internalMutex;

    static QMutex mutex;
    static QAtomicPointer<DataDealSingleton> instance;

    //防止300ms内重复按键
    QTimer* m_popupTrickTimer;
};

#endif // DATADEALSINGLETON_H
