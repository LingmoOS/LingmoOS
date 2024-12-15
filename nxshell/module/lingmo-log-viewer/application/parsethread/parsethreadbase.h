// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PARSETHREADBASE_H
#define PARSETHREADBASE_H
#include "../application/structdef.h"
#include "../application/dbusproxy/dldbushandler.h"
#include "../application/sharedmemorymanager.h"
#include "../application/logfileparser.h"

#include <QProcess>
#include <QRunnable>

/**
 * @brief The ParseThreadKern class 启动日志 内核日志 kwin日志 xorg日志 dpkg日志获取线程
 */
class ParseThreadBase :  public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ParseThreadBase(QObject *parent = nullptr);
    ~ParseThreadBase() override;

    enum Status {
        Normal = 0, //正常结束
        CancelAuth, //取消鉴权
        ForceStop //强制停止
    };

    // 设置筛选条件
    virtual void setFilter(LOG_FILTER_BASE &filter);

    void stopProccess();
    int getIndex();
    /**
     * @brief thread_index 静态成员变量，用来每次构造时标记新的当前线程对象 m_threadIndex
     */
    static int thread_count;

signals:
    /**
     * @brief parseFinished 解析完成信号
     * @param index 当前线程的数字标号
     * @param type 日志种类
     */
    void parseFinished(int index, LOG_FLAG type, ParseThreadBase::Status status = Normal);

    /**
     * @brief logData 日志数据发送信号
     * @param index 当前线程的数字标号
     * @param iDataList　json字串日志数据list
     * @param type 日志种类
     */
    void logData(int index, const QList<QString> &iDataList, LOG_FLAG type);

    void proccessError(const QString &iError);

protected:
    void initConnect();
    void initProccess();

    void run() override = 0;

protected:

    /**
     * @brief m_type 当前线程获取日志数据的类型，用来指定不同的获取逻辑和返回结果
     */
    LOG_FLAG m_type;

    // 解析器对象，用来传递解析数据和信号
    LogFileParser* m_pParser = nullptr;
    /**
     * @brief m_filter 日志筛选条件
     */
    LOG_FILTER_BASE m_filter;

    //获取数据用的cat命令的process
    QScopedPointer<QProcess> m_process;
    /**
     * @brief m_canRun 是否可以继续运行的标记量，用于停止运行线程
     */
    std::atomic_bool m_canRun = false;
    /**
     * @brief m_threadIndex 当前线程标号
     */
    int m_threadCount;
    //正在执行停止进程的变量，防止重复执行停止逻辑
    bool m_isStopProccess = false;
};

Q_DECLARE_METATYPE(ParseThreadBase::Status)

#endif  // PARSETHREADBASE_H
