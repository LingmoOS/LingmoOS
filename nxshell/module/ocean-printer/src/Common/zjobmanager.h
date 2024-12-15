// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZJOBMANAGER_H
#define ZJOBMANAGER_H

#include "cupsattrnames.h"

#include <QList>
#include <QMap>
#include <QVariant>

#include <map>
using namespace std;

#define JOB_ALL "all jobs"

enum {
    WHICH_JOB_ALL = 0,
    WHICH_JOB_RUNING,
    WHICH_JOB_DONE
};

#define LOWEST_Priority 0
#define DEFAULT_Priority 50
#define HIGHEST_Priority 100

#define PrintTestTitle "Test page"

class JobManager : public QObject
{
    Q_OBJECT

public:
    static JobManager *getInstance();

    /*  获取打印任务列表
     * WHICH_JOB_ALL 获取所以任务
     * WHICH_JOB_DONE 获取已完成任务
     * WHICHJOB_RUNING 获取未完成任务
     */
    int getJobs(map<int, map<string, string>> &jobs, int which = WHICH_JOB_RUNING, int myJobs = 0);

    int getJobById(map<string, string> &job, int jobId, int myJob = 1);

    //取消任务
    int cancelJob(int job_id);

    /* 删除任务
     * dest != NULL 删除dest打印机下面所有的任务
     * dest == JOB_ALL 删除面所有的任务
     * dest == NULL 删除job_id指定的打印机
     */
    int deleteJob(int job_id, const char *dest = nullptr);

    //暂停任务
    int holdJob(int job_id);
    int holdjobs(const QString &printerName);

    //唤醒任务
    //int ResumeJob(int job_id);

    //释放任务
    int releaseJob(int job_id);

    //重新开始任务
    int restartJob(int job_id);

    /* 移动任务
     * job_id != 0 移动指定任务
     * job_id == 0 移动src打印机下面所有任务
     */
    int moveJob(const char *destUri, int job_id, const char *srcUri = nullptr);

    int priorityJob(int job_id, int &iPriority);

    //打印测试页
    QString printTestPage(const char *dest, int &jobId, const char *format = nullptr);

    bool isCompletedState(int state);

protected:
    JobManager()
        : QObject(nullptr)
    {
    }
};

#define g_jobManager JobManager::getInstance()

#endif //ZJOBMANAGER_H
