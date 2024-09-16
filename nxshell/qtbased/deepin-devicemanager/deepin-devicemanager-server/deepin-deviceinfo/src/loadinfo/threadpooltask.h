// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREADPOOLTASK_H
#define THREADPOOLTASK_H

#include <QObject>
#include <QRunnable>
#include <QFile>

//#define PATH "/home/liujun/device-info/"
#define PATH "/tmp/device-info/"  // 设备文件存放的目录

/**
 * @brief The ThreadPoolTask class
 */
class ThreadPoolTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ThreadPoolTask(QString cmd, QString file, bool replace, int waiting, QObject *parent = nullptr);
    ~ThreadPoolTask() override;

signals:
    /**
     * @brief finished : finish task
     */
    void finished();

protected:
    void run() override;

private:
    /**
     * @brief runCmd : running a command
     * @param cmd : the command
     */
    void runCmd(const QString &cmd);

    /**
     * @brief runCmd
     * @param cmd
     * @param info
     */
    void runCmd(const QString &cmd, QString &info);

    /**
     * @brief runAsteriskCmd
     * @param cmd
     * @param arg
     * @return info
     */
    QString runAsteriskCmd(const QString &cmd, const QString &arg);

    /**
     * @brief runCmdToCache
     * @param cmd
     */
    void runCmdToCache(const QString &cmd);

    /**
     * @brief loadSmartCtlInfoToCache
     * @param info
     */
    void loadSmartCtlInfoToCache(const QString &info);

    /**
     * @brief loadCpuInfo
     */
    void loadCpuInfo();

    /**
     * @brief loadSgSmartCtlInfoToCache
     * @param info
     */
    void loadSgSmartCtlInfoToCache(const QString &info);

    /**
     * @brief loadLspciVSInfoToCache
     * @param info
     */
    void loadLspciVSInfoToCache(const QString &info);

    /**
     * @brief loadDisplayWidth
     * @param info
     */
    void loadDisplayWidth(const QString &info);

    /**
     * @brief getDisplayWidthFromLspci
     * @param info
     * @return
     */
    int getDisplayWidthFromLspci(const QString &info);

    /**
     * @brief runCmdToTxt
     * @param cmd
     */
    void runCmdToFile(const QString &cmd);

    /**
     * @brief loadSmartctlInfoToFile
     * @param file
     */
    void loadSmartctlInfoToFile(QFile &file);

    /**
     * @brief loadLspciVSInfoToFile
     * @param file
     */
    void loadLspciVSInfoToFile(QFile &file);

private:
    QString   m_Cmd;                  //<! cmd
    QString   m_File;                 //<! file name
    bool      m_CanNotReplace;        //<! Whether to replace if file existed
    int       m_Waiting;              //<! waiting time
};

#endif // THREADPOOLTASK_H
