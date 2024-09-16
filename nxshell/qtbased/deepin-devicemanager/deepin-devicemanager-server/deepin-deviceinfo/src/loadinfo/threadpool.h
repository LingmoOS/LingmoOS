// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QThreadPool>
#include <QList>
#include <QVector>

/**
 * @brief The Cmd struct
 */
struct Cmd {
    Cmd(): cmd(""), file(""), canNotReplace(false), waitingTime(-1)
    {}

    QString cmd;         //<! the cmd
    QString file;        //<! the file
    bool canNotReplace;  //<! mark can replace or not
    int waitingTime;     //<! waiting time
};

/**
 * @brief The ThreadPool class
 */
class ThreadPool : public QThreadPool
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);

    /**
     * @brief generateDeviceFile : load device info
     */
    void loadDeviceInfo();

    /**
     * @brief updateDeviceFile
     */
    void updateDeviceInfo();

private:
    /**
     * @brief runCmdToCache
     * @param cmd
     */
    void runCmdToCache(const Cmd &cmd);

    /**
     * @brief initCmd init all cmd
     */
    void initCmd();

private:
    QList<Cmd>        m_ListCmd;             // all cmd
    QList<Cmd>        m_ListUpdate;          // update cmd
};

#endif // THREADPOOL_H
