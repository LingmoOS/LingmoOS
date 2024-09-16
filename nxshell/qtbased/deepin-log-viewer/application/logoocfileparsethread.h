// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGOOCFILEPARSETHREAD_H
#define LOGOOCFILEPARSETHREAD_H
#include "structdef.h"

#include <QMap>
#include <QObject>
#include <QThread>
#include <QProcess>

#include <mutex>

/**
 * @brief The LogOOCFileParseThread class 应用日志获取线程
 */
class LogOOCFileParseThread : public QThread
{
    Q_OBJECT
public:
    explicit LogOOCFileParseThread(QObject *parent = nullptr);
    ~LogOOCFileParseThread() override;
    void setParam(const QString &path);
    static int thread_count;
    void initProccess();

    const QScopedPointer<QProcess> &getProcess() const {return m_process;}
    const QString &getPath() const {return m_path;}
    const QString &getFileData() const {return m_fileData;}

signals:
    /**
     * @brief sigFinished 获取数据结束信号
     */
    void sigFinished(int index, int error = 0);
    void sigData(int index, const QString &data);
public slots:
    void doWork();
    void stopProccess();
public:
    int getIndex();
protected:
    void run() override;
    //鉴权
    bool checkAuthentication(const QString &path);

private:
    /**
     * @brief m_path 文件路径
     */
    QString m_path;

    /**
     * @brief m_fileData 获取的数据结果
     */
    QString m_fileData;
    /**
     * @brief m_canRun 是否可以继续运行的标记量，用于停止运行线程
     */
    bool m_canRun = false;
    /**
     * @brief m_threadIndex 当前线程标号
     */
    int m_threadCount;

    QScopedPointer<QProcess> m_process;
    //正在执行停止进程的变量，防止重复执行停止逻辑
    bool m_isStopProccess = false;
};

#endif  // LOGAPPLICATIONPARSETHREAD_H
